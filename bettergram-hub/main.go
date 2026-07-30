package main

import (
	"context"
	"database/sql"
	"encoding/json"
	"fmt"
	"log"
	"math/rand"
	"net/http"
	"os"
	"os/signal"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"

	"github.com/go-chi/chi/v5"
	"github.com/go-chi/chi/v5/middleware"
	_ "modernc.org/sqlite"
)

const version = "2.0.0"

const (
	contentTypeJSON   = "application/json"
	contentTypeHeader = "Content-Type"
	errInvalidTgID    = "invalid tg_id"
	errBadJSON        = "bad json"
	routeUser         = "/user/{tg_id}"
)

// ── Types ─────────────────────────────────────────────────────────────────────

type UserProfile struct {
	TgID        int64  `json:"tg_id"`
	AvatarURL   string `json:"avatar_url,omitempty"`
	Color       string `json:"color,omitempty"`
	Badge       string `json:"badge,omitempty"`
	BadgeColor  string `json:"badge_color,omitempty"`
	StatusEmoji string `json:"status_emoji,omitempty"`
	StatusText  string `json:"status_text,omitempty"`
	LastSeen    int64  `json:"last_seen,omitempty"`
	UpdatedAt   int64  `json:"updated_at"`
}

type RelayEntry struct {
	ID        string `json:"id"`
	URL       string `json:"url"`
	Filename  string `json:"filename,omitempty"`
	Size      int64  `json:"size,omitempty"`
	Mime      string `json:"mime,omitempty"`
	ShortURL  string `json:"short_url,omitempty"`
	CreatedAt int64  `json:"created_at,omitempty"`
}

type Reaction struct {
	TgID  int64  `json:"tg_id"`
	Emoji string `json:"emoji"`
	Ts    int64  `json:"ts"`
}

type Announcement struct {
	ID        string `json:"id"`
	Title     string `json:"title"`
	Body      string `json:"body,omitempty"`
	Pinned    bool   `json:"pinned"`
	CreatedAt int64  `json:"created_at"`
}

type HubTheme struct {
	Name    string `json:"name"`
	Bg      string `json:"bg"`
	Surface string `json:"surface"`
	Card    string `json:"card"`
	MsgIn   string `json:"msg_in"`
	MsgOut  string `json:"msg_out"`
	Accent  string `json:"accent"`
	Text    string `json:"text"`
	TextSub string `json:"text_sub"`
}

type HubInfo struct {
	Name     string   `json:"name"`
	Version  string   `json:"version"`
	Features []string `json:"features"`
	Users    int      `json:"users"`
}

// ── DB ────────────────────────────────────────────────────────────────────────

var db *sql.DB

func initDB(path string) {
	var err error
	db, err = sql.Open("sqlite", path)
	if err != nil {
		log.Fatal(err)
	}
	// WAL mode: dramatically better concurrent read performance
	db.SetMaxOpenConns(1) // SQLite single writer
	if _, err := db.Exec("PRAGMA journal_mode=WAL"); err != nil {
		log.Printf("warn: could not enable WAL mode: %v", err)
	}
	if _, err := db.Exec("PRAGMA synchronous=NORMAL"); err != nil {
		log.Printf("warn: could not set synchronous=NORMAL: %v", err)
	}
	if _, err := db.Exec("PRAGMA foreign_keys=ON"); err != nil {
		log.Printf("warn: could not enable foreign keys: %v", err)
	}

	_, err = db.Exec(`
		CREATE TABLE IF NOT EXISTS users (
			tg_id        INTEGER PRIMARY KEY,
			avatar_url   TEXT    NOT NULL DEFAULT '',
			color        TEXT    NOT NULL DEFAULT '',
			badge        TEXT    NOT NULL DEFAULT '',
			badge_color  TEXT    NOT NULL DEFAULT '',
			status_emoji TEXT    NOT NULL DEFAULT '',
			status_text  TEXT    NOT NULL DEFAULT '',
			last_seen    INTEGER NOT NULL DEFAULT 0,
			updated_at   INTEGER NOT NULL
		);
		CREATE TABLE IF NOT EXISTS relay (
			id         TEXT    PRIMARY KEY,
			url        TEXT    NOT NULL,
			filename   TEXT    NOT NULL DEFAULT '',
			size       INTEGER NOT NULL DEFAULT 0,
			mime       TEXT    NOT NULL DEFAULT '',
			created_at INTEGER NOT NULL
		);
		CREATE TABLE IF NOT EXISTS reactions (
			peer_id INTEGER NOT NULL,
			msg_id  INTEGER NOT NULL,
			tg_id   INTEGER NOT NULL,
			emoji   TEXT    NOT NULL,
			ts      INTEGER NOT NULL,
			PRIMARY KEY (peer_id, msg_id, tg_id)
		);
		CREATE TABLE IF NOT EXISTS announcements (
			id         TEXT    PRIMARY KEY,
			title      TEXT    NOT NULL,
			body       TEXT    NOT NULL DEFAULT '',
			pinned     INTEGER NOT NULL DEFAULT 0,
			created_at INTEGER NOT NULL
		);
		CREATE TABLE IF NOT EXISTS settings (
			key   TEXT PRIMARY KEY,
			value TEXT NOT NULL
		);
	`)
	if err != nil {
		log.Fatal(err)
	}

	// Migrations: add new columns for existing databases
	migrations := []string{
		"ALTER TABLE users ADD COLUMN badge_color TEXT NOT NULL DEFAULT ''",
		"ALTER TABLE users ADD COLUMN last_seen   INTEGER NOT NULL DEFAULT 0",
		"ALTER TABLE users ADD COLUMN status_emoji TEXT NOT NULL DEFAULT ''",
		"ALTER TABLE users ADD COLUMN status_text  TEXT NOT NULL DEFAULT ''",
	}
	for _, m := range migrations {
		db.Exec(m) // ignore error — column may already exist
	}
}

// ── Rate limiter ─────────────────────────────────────────────────────────────
// Simple per-IP sliding window: max 120 requests per minute.

type ipBucket struct {
	mu      sync.Mutex
	count   int
	resetAt time.Time
}

var (
	rateMu  sync.Mutex
	buckets = map[string]*ipBucket{}
)

func rateLimit(next http.Handler) http.Handler {
	const maxPerMin = 120
	// Prune stale buckets every 5 minutes
	go func() {
		for range time.Tick(5 * time.Minute) {
			now := time.Now()
			rateMu.Lock()
			for ip, b := range buckets {
				b.mu.Lock()
				if now.After(b.resetAt) {
					delete(buckets, ip)
				}
				b.mu.Unlock()
			}
			rateMu.Unlock()
		}
	}()

	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		ip := r.RemoteAddr
		if xff := r.Header.Get("X-Forwarded-For"); xff != "" {
			ip = strings.Split(xff, ",")[0]
		}

		rateMu.Lock()
		b, ok := buckets[ip]
		if !ok {
			b = &ipBucket{}
			buckets[ip] = b
		}
		rateMu.Unlock()

		b.mu.Lock()
		now := time.Now()
		if now.After(b.resetAt) {
			b.count = 0
			b.resetAt = now.Add(time.Minute)
		}
		b.count++
		allowed := b.count <= maxPerMin
		b.mu.Unlock()

		if !allowed {
			w.Header().Set("Retry-After", "60")
			writeErr(w, http.StatusTooManyRequests, "rate limit exceeded")
			return
		}
		next.ServeHTTP(w, r)
	})
}

// ── Helpers ───────────────────────────────────────────────────────────────────

var letters = []byte("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")

func randID(n int) string {
	b := make([]byte, n)
	for i := range b {
		b[i] = letters[rand.Intn(len(letters))]
	}
	return string(b)
}

func hubBase() string {
	if v := os.Getenv("HUB_URL"); v != "" {
		return strings.TrimRight(v, "/")
	}
	return "http://localhost:8080"
}

func hubName() string {
	if v := os.Getenv("HUB_NAME"); v != "" {
		return v
	}
	return "BetterGram Hub"
}

func writeJSON(w http.ResponseWriter, v any) {
	w.Header().Set(contentTypeHeader, contentTypeJSON)
	json.NewEncoder(w).Encode(v)
}

func writeErr(w http.ResponseWriter, code int, msg string) {
	w.Header().Set(contentTypeHeader, contentTypeJSON)
	w.WriteHeader(code)
	json.NewEncoder(w).Encode(map[string]string{"error": msg})
}

func adminToken() string { return os.Getenv("ADMIN_TOKEN") }

func requireAdmin(w http.ResponseWriter, r *http.Request) bool {
	token := adminToken()
	if token == "" {
		writeErr(w, http.StatusForbidden, "admin routes disabled (set ADMIN_TOKEN env var)")
		return false
	}
	if r.Header.Get("X-Admin-Token") != token {
		writeErr(w, http.StatusUnauthorized, "invalid admin token")
		return false
	}
	return true
}

// ── Handlers: meta ────────────────────────────────────────────────────────────

func handleInfo(w http.ResponseWriter, r *http.Request) {
	var count int
	db.QueryRow("SELECT COUNT(*) FROM users").Scan(&count)
	writeJSON(w, HubInfo{
		Name:    hubName(),
		Version: version,
		Features: []string{
			"profiles", "relay", "reactions",
			"announcements", "presence", "hub-theme", "bulk-fetch",
		},
		Users: count,
	})
}

func handleHealth(w http.ResponseWriter, r *http.Request) {
	if err := db.Ping(); err != nil {
		writeErr(w, http.StatusServiceUnavailable, "db unavailable")
		return
	}
	w.WriteHeader(http.StatusOK)
	w.Write([]byte(`{"status":"ok"}`))
}

// ── Handlers: users ───────────────────────────────────────────────────────────

func scanProfile(row interface{ Scan(...any) error }) (UserProfile, error) {
	var p UserProfile
	return p, row.Scan(
		&p.TgID, &p.AvatarURL, &p.Color, &p.Badge, &p.BadgeColor,
		&p.StatusEmoji, &p.StatusText, &p.LastSeen, &p.UpdatedAt,
	)
}

const selectUser = `SELECT tg_id, avatar_url, color, badge, badge_color,
	status_emoji, status_text, last_seen, updated_at FROM users`

func handleListUsers(w http.ResponseWriter, r *http.Request) {
	rows, err := db.Query("SELECT tg_id FROM users ORDER BY updated_at DESC")
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	defer rows.Close()
	ids := []int64{}
	for rows.Next() {
		var id int64
		rows.Scan(&id)
		ids = append(ids, id)
	}
	writeJSON(w, ids)
}

func handleGetUser(w http.ResponseWriter, r *http.Request) {
	id, err := strconv.ParseInt(chi.URLParam(r, "tg_id"), 10, 64)
	if err != nil {
		writeErr(w, 400, errInvalidTgID)
		return
	}
	p, err := scanProfile(db.QueryRow(selectUser+` WHERE tg_id = ?`, id))
	if err == sql.ErrNoRows {
		writeErr(w, 404, "not found")
		return
	}
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	writeJSON(w, p)
}

// POST /users/batch  body: {"ids":[1,2,3,...]}
// Returns a map of tg_id → profile for all found users. Missing IDs are omitted.
func handleBatchUsers(w http.ResponseWriter, r *http.Request) {
	var req struct {
		IDs []int64 `json:"ids"`
	}
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil || len(req.IDs) == 0 {
		writeErr(w, 400, "ids array required")
		return
	}
	if len(req.IDs) > 200 {
		writeErr(w, 400, "max 200 ids per request")
		return
	}

	// Build IN clause
	placeholders := make([]string, len(req.IDs))
	args := make([]any, len(req.IDs))
	for i, id := range req.IDs {
		placeholders[i] = "?"
		args[i] = id
	}
	query := selectUser + ` WHERE tg_id IN (` + strings.Join(placeholders, ",") + `)`
	rows, err := db.Query(query, args...)
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	defer rows.Close()

	result := map[string]UserProfile{}
	for rows.Next() {
		p, err := scanProfile(rows)
		if err == nil {
			result[strconv.FormatInt(p.TgID, 10)] = p
		}
	}
	writeJSON(w, result)
}

func handleSetUser(w http.ResponseWriter, r *http.Request) {
	id, err := strconv.ParseInt(chi.URLParam(r, "tg_id"), 10, 64)
	if err != nil {
		writeErr(w, 400, errInvalidTgID)
		return
	}
	var p UserProfile
	if err := json.NewDecoder(r.Body).Decode(&p); err != nil {
		writeErr(w, 400, errBadJSON)
		return
	}
	p.TgID = id
	p.UpdatedAt = time.Now().Unix()

	_, err = db.Exec(
		`INSERT INTO users (tg_id, avatar_url, color, badge, badge_color, status_emoji, status_text, last_seen, updated_at)
		 VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
		 ON CONFLICT(tg_id) DO UPDATE SET
		   avatar_url=excluded.avatar_url,
		   color=excluded.color,
		   status_emoji=excluded.status_emoji,
		   status_text=excluded.status_text,
		   updated_at=excluded.updated_at`,
		p.TgID, p.AvatarURL, p.Color, p.Badge, p.BadgeColor,
		p.StatusEmoji, p.StatusText, p.LastSeen, p.UpdatedAt,
	)
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	writeJSON(w, p)
}

func handleDeleteUser(w http.ResponseWriter, r *http.Request) {
	id, _ := strconv.ParseInt(chi.URLParam(r, "tg_id"), 10, 64)
	db.Exec("DELETE FROM users WHERE tg_id = ?", id)
	w.WriteHeader(204)
}

// POST /heartbeat/{tg_id} — update presence timestamp (no body needed)
func handleHeartbeat(w http.ResponseWriter, r *http.Request) {
	id, err := strconv.ParseInt(chi.URLParam(r, "tg_id"), 10, 64)
	if err != nil {
		writeErr(w, 400, errInvalidTgID)
		return
	}
	now := time.Now().Unix()
	_, err = db.Exec(
		`INSERT INTO users (tg_id, last_seen, updated_at)
		 VALUES (?, ?, ?)
		 ON CONFLICT(tg_id) DO UPDATE SET last_seen=excluded.last_seen`,
		id, now, now,
	)
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	w.WriteHeader(204)
}

// ── Handlers: admin roles ─────────────────────────────────────────────────────

func handleAdminSetRole(w http.ResponseWriter, r *http.Request) {
	if !requireAdmin(w, r) {
		return
	}
	id, err := strconv.ParseInt(chi.URLParam(r, "tg_id"), 10, 64)
	if err != nil {
		writeErr(w, 400, errInvalidTgID)
		return
	}
	var body struct {
		Badge      string `json:"badge"`
		BadgeColor string `json:"badge_color"`
	}
	if err := json.NewDecoder(r.Body).Decode(&body); err != nil {
		writeErr(w, 400, errBadJSON)
		return
	}
	_, err = db.Exec(
		`INSERT INTO users (tg_id, avatar_url, color, badge, badge_color, status_emoji, status_text, last_seen, updated_at)
		 VALUES (?, '', '', ?, ?, '', '', 0, ?)
		 ON CONFLICT(tg_id) DO UPDATE SET
		   badge=excluded.badge,
		   badge_color=excluded.badge_color,
		   updated_at=excluded.updated_at`,
		id, body.Badge, body.BadgeColor, time.Now().Unix(),
	)
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	w.WriteHeader(204)
}

func handleAdminRemoveRole(w http.ResponseWriter, r *http.Request) {
	if !requireAdmin(w, r) {
		return
	}
	id, err := strconv.ParseInt(chi.URLParam(r, "tg_id"), 10, 64)
	if err != nil {
		writeErr(w, 400, errInvalidTgID)
		return
	}
	db.Exec(`UPDATE users SET badge='', badge_color='', updated_at=? WHERE tg_id=?`,
		time.Now().Unix(), id)
	w.WriteHeader(204)
}

func handleAdminListRoles(w http.ResponseWriter, r *http.Request) {
	if !requireAdmin(w, r) {
		return
	}
	rows, err := db.Query(
		`SELECT tg_id, badge, badge_color FROM users WHERE badge != '' ORDER BY tg_id`)
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	defer rows.Close()
	type RoleEntry struct {
		TgID       int64  `json:"tg_id"`
		Badge      string `json:"badge"`
		BadgeColor string `json:"badge_color"`
	}
	list := []RoleEntry{}
	for rows.Next() {
		var e RoleEntry
		rows.Scan(&e.TgID, &e.Badge, &e.BadgeColor)
		list = append(list, e)
	}
	writeJSON(w, list)
}

// ── Handlers: hub theme ───────────────────────────────────────────────────────

// GET /theme — returns the hub's pushed theme, or 404 if none is set.
// BetterGram clients can poll this to auto-apply the hub owner's chosen theme.
func handleGetTheme(w http.ResponseWriter, r *http.Request) {
	var raw string
	err := db.QueryRow(`SELECT value FROM settings WHERE key='hub_theme'`).Scan(&raw)
	if err == sql.ErrNoRows {
		writeErr(w, 404, "no hub theme set")
		return
	}
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	w.Header().Set(contentTypeHeader, contentTypeJSON)
	w.Write([]byte(raw))
}

// PUT /admin/theme — set the hub theme (admin-only). Body: HubTheme JSON.
func handleAdminSetTheme(w http.ResponseWriter, r *http.Request) {
	if !requireAdmin(w, r) {
		return
	}
	var t HubTheme
	if err := json.NewDecoder(r.Body).Decode(&t); err != nil {
		writeErr(w, 400, errBadJSON)
		return
	}
	if t.Name == "" {
		writeErr(w, 400, "name required")
		return
	}
	raw, _ := json.Marshal(t)
	_, err := db.Exec(
		`INSERT INTO settings (key, value) VALUES ('hub_theme', ?)
		 ON CONFLICT(key) DO UPDATE SET value=excluded.value`,
		string(raw),
	)
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	w.Header().Set(contentTypeHeader, contentTypeJSON)
	w.Write(raw)
}

// DELETE /admin/theme — clear the hub theme.
func handleAdminDeleteTheme(w http.ResponseWriter, r *http.Request) {
	if !requireAdmin(w, r) {
		return
	}
	db.Exec(`DELETE FROM settings WHERE key='hub_theme'`)
	w.WriteHeader(204)
}

// ── Handlers: announcements ───────────────────────────────────────────────────

// GET /announcements — returns all announcements, pinned first.
func handleListAnnouncements(w http.ResponseWriter, r *http.Request) {
	rows, err := db.Query(
		`SELECT id, title, body, pinned, created_at
		 FROM announcements ORDER BY pinned DESC, created_at DESC`)
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	defer rows.Close()
	list := []Announcement{}
	for rows.Next() {
		var a Announcement
		var pinned int
		rows.Scan(&a.ID, &a.Title, &a.Body, &pinned, &a.CreatedAt)
		a.Pinned = pinned == 1
		list = append(list, a)
	}
	writeJSON(w, list)
}

// POST /admin/announcements — create an announcement (admin-only).
func handleAdminPostAnnouncement(w http.ResponseWriter, r *http.Request) {
	if !requireAdmin(w, r) {
		return
	}
	var a Announcement
	if err := json.NewDecoder(r.Body).Decode(&a); err != nil {
		writeErr(w, 400, errBadJSON)
		return
	}
	if strings.TrimSpace(a.Title) == "" {
		writeErr(w, 400, "title required")
		return
	}
	a.ID = randID(8)
	a.CreatedAt = time.Now().Unix()
	pinned := 0
	if a.Pinned {
		pinned = 1
	}
	_, err := db.Exec(
		`INSERT INTO announcements (id, title, body, pinned, created_at) VALUES (?, ?, ?, ?, ?)`,
		a.ID, a.Title, a.Body, pinned, a.CreatedAt,
	)
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	w.WriteHeader(201)
	writeJSON(w, a)
}

// DELETE /admin/announcements/{id} — remove an announcement.
func handleAdminDeleteAnnouncement(w http.ResponseWriter, r *http.Request) {
	if !requireAdmin(w, r) {
		return
	}
	id := chi.URLParam(r, "id")
	db.Exec(`DELETE FROM announcements WHERE id=?`, id)
	w.WriteHeader(204)
}

// ── Handlers: relay ───────────────────────────────────────────────────────────

func handleRegisterRelay(w http.ResponseWriter, r *http.Request) {
	var e RelayEntry
	if err := json.NewDecoder(r.Body).Decode(&e); err != nil {
		writeErr(w, 400, errBadJSON)
		return
	}
	if e.URL == "" {
		writeErr(w, 400, "url required")
		return
	}
	e.ID = randID(8)
	e.CreatedAt = time.Now().Unix()
	e.ShortURL = fmt.Sprintf("%s/r/%s", hubBase(), e.ID)

	_, err := db.Exec(
		`INSERT INTO relay (id, url, filename, size, mime, created_at) VALUES (?, ?, ?, ?, ?, ?)`,
		e.ID, e.URL, e.Filename, e.Size, e.Mime, e.CreatedAt,
	)
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	w.WriteHeader(201)
	writeJSON(w, e)
}

func handleGetRelay(w http.ResponseWriter, r *http.Request) {
	id := chi.URLParam(r, "id")
	var e RelayEntry
	err := db.QueryRow(
		`SELECT id, url, filename, size, mime, created_at FROM relay WHERE id = ?`, id,
	).Scan(&e.ID, &e.URL, &e.Filename, &e.Size, &e.Mime, &e.CreatedAt)
	if err == sql.ErrNoRows {
		writeErr(w, 404, "not found")
		return
	}
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	e.ShortURL = fmt.Sprintf("%s/r/%s", hubBase(), e.ID)
	writeJSON(w, e)
}

func handleShortRedirect(w http.ResponseWriter, r *http.Request) {
	id := chi.URLParam(r, "id")
	var url string
	err := db.QueryRow("SELECT url FROM relay WHERE id = ?", id).Scan(&url)
	if err == sql.ErrNoRows {
		http.NotFound(w, r)
		return
	}
	http.Redirect(w, r, url, http.StatusFound)
}

// ── Handlers: reactions ───────────────────────────────────────────────────────

func handleGetReactions(w http.ResponseWriter, r *http.Request) {
	peerID, _ := strconv.ParseInt(chi.URLParam(r, "peer_id"), 10, 64)
	msgID, _ := strconv.ParseInt(chi.URLParam(r, "msg_id"), 10, 64)
	rows, err := db.Query(
		`SELECT tg_id, emoji, ts FROM reactions WHERE peer_id=? AND msg_id=? ORDER BY ts ASC`,
		peerID, msgID,
	)
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	defer rows.Close()
	reacts := []Reaction{}
	for rows.Next() {
		var rx Reaction
		rows.Scan(&rx.TgID, &rx.Emoji, &rx.Ts)
		reacts = append(reacts, rx)
	}
	writeJSON(w, reacts)
}

func handleAddReaction(w http.ResponseWriter, r *http.Request) {
	peerID, _ := strconv.ParseInt(chi.URLParam(r, "peer_id"), 10, 64)
	msgID, _ := strconv.ParseInt(chi.URLParam(r, "msg_id"), 10, 64)
	var rx Reaction
	if err := json.NewDecoder(r.Body).Decode(&rx); err != nil {
		writeErr(w, 400, errBadJSON)
		return
	}
	if rx.TgID == 0 || rx.Emoji == "" {
		writeErr(w, 400, "tg_id and emoji required")
		return
	}
	rx.Ts = time.Now().Unix()
	_, err := db.Exec(
		`INSERT INTO reactions (peer_id, msg_id, tg_id, emoji, ts)
		 VALUES (?, ?, ?, ?, ?)
		 ON CONFLICT(peer_id, msg_id, tg_id) DO UPDATE SET emoji=excluded.emoji, ts=excluded.ts`,
		peerID, msgID, rx.TgID, rx.Emoji, rx.Ts,
	)
	if err != nil {
		writeErr(w, 500, err.Error())
		return
	}
	writeJSON(w, rx)
}

func handleRemoveReaction(w http.ResponseWriter, r *http.Request) {
	peerID, _ := strconv.ParseInt(chi.URLParam(r, "peer_id"), 10, 64)
	msgID, _ := strconv.ParseInt(chi.URLParam(r, "msg_id"), 10, 64)
	tgID, _ := strconv.ParseInt(chi.URLParam(r, "tg_id"), 10, 64)
	db.Exec(`DELETE FROM reactions WHERE peer_id=? AND msg_id=? AND tg_id=?`, peerID, msgID, tgID)
	w.WriteHeader(204)
}

// ── Main ──────────────────────────────────────────────────────────────────────

func main() {
	dbPath := os.Getenv("DB_PATH")
	if dbPath == "" {
		dbPath = "hub.db"
	}
	port := os.Getenv("PORT")
	if port == "" {
		port = "8080"
	}

	initDB(dbPath)

	r := chi.NewRouter()
	r.Use(middleware.RealIP)
	r.Use(middleware.Logger)
	r.Use(middleware.Recoverer)
	r.Use(rateLimit)
	r.Use(func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			w.Header().Set("Access-Control-Allow-Origin", "*")
			w.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
			w.Header().Set("Access-Control-Allow-Headers", "Content-Type, X-Admin-Token")
			if r.Method == http.MethodOptions {
				w.WriteHeader(204)
				return
			}
			next.ServeHTTP(w, r)
		})
	})

	// ── Meta
	r.Get("/health", handleHealth)
	r.Get("/info", handleInfo)

	// ── Users
	r.Get("/users", handleListUsers)
	r.Post("/users/batch", handleBatchUsers)
	r.Get(routeUser, handleGetUser)
	r.Put(routeUser, handleSetUser)
	r.Delete(routeUser, handleDeleteUser)
	r.Post("/heartbeat/{tg_id}", handleHeartbeat)

	// ── Relay
	r.Post("/relay", handleRegisterRelay)
	r.Get("/relay/{id}", handleGetRelay)
	r.Get("/r/{id}", handleShortRedirect)

	// ── Reactions
	r.Get("/reactions/{peer_id}/{msg_id}", handleGetReactions)
	r.Post("/reactions/{peer_id}/{msg_id}", handleAddReaction)
	r.Delete("/reactions/{peer_id}/{msg_id}/{tg_id}", handleRemoveReaction)

	// ── Hub theme
	r.Get("/theme", handleGetTheme)

	// ── Announcements
	r.Get("/announcements", handleListAnnouncements)

	// ── Admin (require X-Admin-Token)
	r.Get("/admin/roles", handleAdminListRoles)
	r.Put("/admin/role/{tg_id}", handleAdminSetRole)
	r.Delete("/admin/role/{tg_id}", handleAdminRemoveRole)
	r.Put("/admin/theme", handleAdminSetTheme)
	r.Delete("/admin/theme", handleAdminDeleteTheme)
	r.Post("/admin/announcements", handleAdminPostAnnouncement)
	r.Delete("/admin/announcements/{id}", handleAdminDeleteAnnouncement)

	srv := &http.Server{
		Addr:         ":" + port,
		Handler:      r,
		ReadTimeout:  15 * time.Second,
		WriteTimeout: 15 * time.Second,
		IdleTimeout:  60 * time.Second,
	}

	log.Printf("BetterGram Hub v%s starting on :%s (db: %s)", version, port, dbPath)

	go func() {
		if err := srv.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			log.Fatalf("listen: %v", err)
		}
	}()

	// Graceful shutdown on SIGINT / SIGTERM
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit

	log.Println("shutting down...")
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()
	if err := srv.Shutdown(ctx); err != nil {
		log.Printf("shutdown error: %v", err)
	}
	db.Close()
	log.Println("bye")
}
