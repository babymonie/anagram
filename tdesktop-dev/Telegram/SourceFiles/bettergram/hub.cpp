#include "bettergram/hub.h"

#include "base/unixtime.h"
#include "core/application.h"
#include "core/core_settings.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace BetterGram {
namespace {

constexpr auto kUserListRefreshMs   = 10 * 60 * 1000; // 10 min
constexpr auto kHeartbeatIntervalMs =  5 * 60 * 1000; //  5 min

const QStringList kImageExts = { "jpg","jpeg","png","gif","webp","avif","bmp" };
const QStringList kVideoExts = { "mp4","webm","mov","avi","mkv","m4v" };
const QStringList kAudioExts = { "mp3","ogg","m4a","flac","wav","opus" };

} // namespace

// ── HubRelay helpers ─────────────────────────────────────────────────────────

bool HubRelay::isImage() const {
	const auto ext = filename.section('.', -1).toLower();
	return kImageExts.contains(ext)
		|| mime.startsWith("image/");
}

bool HubRelay::isVideo() const {
	const auto ext = filename.section('.', -1).toLower();
	return kVideoExts.contains(ext)
		|| mime.startsWith("video/");
}

bool HubRelay::isAudio() const {
	const auto ext = filename.section('.', -1).toLower();
	return kAudioExts.contains(ext)
		|| mime.startsWith("audio/");
}

// ── Hub singleton ─────────────────────────────────────────────────────────────

Hub &Hub::instance() {
	static Hub inst;
	return inst;
}

Hub::Hub(QObject *parent) : QObject(parent) {
	_net = new QNetworkAccessManager(this);

	// Refresh user list periodically so badge detection works
	const auto timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [this] { refreshUserList(); });
	timer->start(kUserListRefreshMs);

	// Defer the initial settings read to the next event loop iteration.
	// Hub::instance() (a lazily-constructed singleton) can be first reached
	// before Core::Application finishes initialising, when Core::IsAppLaunched()
	// is still false and Core::App() would be unsafe to call. Reading it
	// synchronously here meant _hubUrl could stay empty forever even after
	// the user had already configured a hub URL in a previous session. By
	// the time the event loop runs this callback, the app is launched.
	QTimer::singleShot(0, this, [this] {
		if (Core::IsAppLaunched()) {
			_hubUrl = Core::App().settings().bettergramHubUrl();
		}
		if (isConfigured()) {
			refreshUserList();
		}
	});
}

// ── Configuration ─────────────────────────────────────────────────────────────

void Hub::setHubUrl(const QString &url) {
	_hubUrl = url.trimmed();
	if (_hubUrl.endsWith('/')) {
		_hubUrl.chop(1);
	}
	Core::App().settings().setBettergramHubUrl(_hubUrl);
	Core::App().saveSettingsDelayed();
	_knownUsers.clear();
	_userListFresh = false;
	if (isConfigured()) {
		refreshUserList();
	}
}

QString Hub::hubUrl() const {
	return _hubUrl;
}

bool Hub::isConfigured() const {
	return !_hubUrl.isEmpty();
}

// ── HTTP helpers ──────────────────────────────────────────────────────────────

void Hub::get(const QString &path, Fn<void(QJsonDocument)> callback) {
	if (!isConfigured()) return;
	const auto req = QNetworkRequest(QUrl(_hubUrl + path));
	auto *reply = _net->get(req);
	connect(reply, &QNetworkReply::finished, this, [reply, callback] {
		const auto data = reply->readAll();
		reply->deleteLater();
		callback(QJsonDocument::fromJson(data));
	});
}

void Hub::post(
		const QString &path,
		const QByteArray &body,
		Fn<void(QJsonDocument, int)> callback) {
	if (!isConfigured()) return;
	QNetworkRequest req(QUrl(_hubUrl + path));
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	auto *reply = _net->post(req, body);
	connect(reply, &QNetworkReply::finished, this, [reply, callback] {
		const int status = reply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();
		const auto data = reply->readAll();
		reply->deleteLater();
		callback(QJsonDocument::fromJson(data), status);
	});
}

void Hub::put(
		const QString &path,
		const QByteArray &body,
		Fn<void(QJsonDocument, int)> callback) {
	if (!isConfigured()) return;
	QNetworkRequest req(QUrl(_hubUrl + path));
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	auto *reply = _net->put(req, body);
	connect(reply, &QNetworkReply::finished, this, [reply, callback] {
		const int status = reply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();
		const auto data = reply->readAll();
		reply->deleteLater();
		callback(QJsonDocument::fromJson(data), status);
	});
}

void Hub::del(const QString &path, Fn<void(int)> callback) {
	if (!isConfigured()) return;
	const auto req = QNetworkRequest(QUrl(_hubUrl + path));
	auto *reply = _net->deleteResource(req);
	connect(reply, &QNetworkReply::finished, this, [reply, callback] {
		const int status = reply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();
		reply->deleteLater();
		if (callback) callback(status);
	});
}

void Hub::putAdmin(
		const QString &path,
		const QByteArray &body,
		const QString &token,
		Fn<void(int)> callback) {
	if (!isConfigured()) return;
	QNetworkRequest req(QUrl(_hubUrl + path));
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	req.setRawHeader("X-Admin-Token", token.toUtf8());
	auto *reply = _net->put(req, body);
	connect(reply, &QNetworkReply::finished, this, [reply, callback] {
		const int status = reply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();
		reply->deleteLater();
		if (callback) callback(status);
	});
}

void Hub::delAdmin(
		const QString &path,
		const QString &token,
		Fn<void(int)> callback) {
	if (!isConfigured()) return;
	QNetworkRequest req(QUrl(_hubUrl + path));
	req.setRawHeader("X-Admin-Token", token.toUtf8());
	auto *reply = _net->deleteResource(req);
	connect(reply, &QNetworkReply::finished, this, [reply, callback] {
		const int status = reply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();
		reply->deleteLater();
		if (callback) callback(status);
	});
}

// ── Profiles ──────────────────────────────────────────────────────────────────

void Hub::fetchProfile(
		int64 tgId,
		Fn<void(std::optional<HubProfile>)> callback) {
	// Serve from cache if fresh
	if (const auto it = _profileCache.find(tgId); it != _profileCache.end()) {
		if (crl::now() - it->fetchTime < kProfileCacheTtl) {
			callback(it->profile);
			return;
		}
	}
	get(u"/user/%1"_q.arg(tgId), [=](QJsonDocument doc) {
		if (doc.isNull() || !doc.object().contains("tg_id")) {
			callback(std::nullopt);
			return;
		}
		const auto obj = doc.object();
		HubProfile p;
		p.tgId       = obj["tg_id"].toVariant().toLongLong();
		p.avatarUrl  = obj["avatar_url"].toString();
		p.color      = obj["color"].toString();
		p.badge      = obj["badge"].toString();
		p.badgeColor = obj["badge_color"].toString();
		p.statusEmoji = obj["status_emoji"].toString();
		p.statusText = obj["status_text"].toString();
		p.lastSeen = obj["last_seen"].toVariant().toLongLong();
		_profileCache[tgId] = { p, crl::now() };
		callback(p);
	});
}

void Hub::setMyProfile(
		const HubProfile &profile,
		Fn<void(bool)> callback) {
	QJsonObject obj;
	obj["avatar_url"]   = profile.avatarUrl;
	obj["color"]        = profile.color;
	obj["status_emoji"] = profile.statusEmoji;
	obj["status_text"]  = profile.statusText;
	const auto body = QJsonDocument(obj).toJson(QJsonDocument::Compact);
	put(u"/user/%1"_q.arg(profile.tgId), body, [=](QJsonDocument, int status) {
		if (callback) callback(status >= 200 && status < 300);
	});
}

bool Hub::isKnownBgUser(int64 tgId) const {
	return _knownUsers.contains(tgId);
}

void Hub::refreshUserList() {
	get(u"/users"_q, [=](QJsonDocument doc) {
		if (!doc.isArray()) return;
		_knownUsers.clear();
		for (const auto &v : doc.array()) {
			_knownUsers.insert(v.toVariant().toLongLong());
		}
		_userListFresh = true;
	});
}

// ── Relay ─────────────────────────────────────────────────────────────────────

void Hub::registerRelay(
		const QString &externalUrl,
		const QString &filename,
		int64 size,
		const QString &mime,
		Fn<void(std::optional<HubRelay>)> callback) {
	QJsonObject obj;
	obj["url"]      = externalUrl;
	obj["filename"] = filename;
	obj["size"]     = size;
	obj["mime"]     = mime;
	const auto body = QJsonDocument(obj).toJson(QJsonDocument::Compact);
	post(u"/relay"_q, body, [=](QJsonDocument doc, int status) {
		if (status != 201 || doc.isNull()) {
			callback(std::nullopt);
			return;
		}
		const auto o = doc.object();
		HubRelay r;
		r.id       = o["id"].toString();
		r.url      = o["url"].toString();
		r.filename = o["filename"].toString();
		r.size     = o["size"].toVariant().toLongLong();
		r.mime     = o["mime"].toString();
		r.shortUrl = o["short_url"].toString();
		callback(r);
	});
}

void Hub::resolveRelay(
		const QString &id,
		Fn<void(std::optional<HubRelay>)> callback) {
	get(u"/relay/%1"_q.arg(id), [=](QJsonDocument doc) {
		if (doc.isNull() || !doc.object().contains("id")) {
			callback(std::nullopt);
			return;
		}
		const auto o = doc.object();
		HubRelay r;
		r.id       = o["id"].toString();
		r.url      = o["url"].toString();
		r.filename = o["filename"].toString();
		r.size     = o["size"].toVariant().toLongLong();
		r.mime     = o["mime"].toString();
		r.shortUrl = o["short_url"].toString();
		callback(r);
	});
}

// ── Reactions ─────────────────────────────────────────────────────────────────

void Hub::addReaction(
		int64 peerId,
		int32 msgId,
		int64 myTgId,
		const QString &emoji) {
	QJsonObject obj;
	obj["tg_id"] = myTgId;
	obj["emoji"] = emoji;
	const auto body = QJsonDocument(obj).toJson(QJsonDocument::Compact);
	const auto path = u"/reactions/%1/%2"_q.arg(peerId).arg(msgId);
	post(path, body, [](QJsonDocument, int) {});
}

void Hub::removeReaction(int64 peerId, int32 msgId, int64 myTgId) {
	del(u"/reactions/%1/%2/%3"_q.arg(peerId).arg(msgId).arg(myTgId));
}

void Hub::fetchReactions(
		int64 peerId,
		int32 msgId,
		Fn<void(QVector<HubReaction>)> callback) {
	get(u"/reactions/%1/%2"_q.arg(peerId).arg(msgId), [=](QJsonDocument doc) {
		QVector<HubReaction> result;
		if (!doc.isArray()) {
			callback(result);
			return;
		}
		for (const auto &v : doc.array()) {
			const auto o = v.toObject();
			HubReaction rx;
			rx.tgId  = o["tg_id"].toVariant().toLongLong();
			rx.emoji = o["emoji"].toString();
			rx.ts    = o["ts"].toVariant().toLongLong();
			result.push_back(rx);
		}
		callback(result);
	});
}

// ── Cached badge lookup ───────────────────────────────────────────────────────

std::optional<std::pair<QString, QString>> Hub::cachedBadge(int64 tgId) const {
	if (const auto it = _profileCache.find(tgId); it != _profileCache.end()) {
		if (!it->profile.badge.isEmpty()) {
			return std::make_pair(it->profile.badge, it->profile.badgeColor);
		}
		return std::nullopt; // cached but no badge
	}
	return std::nullopt; // not cached
}

std::optional<int64> Hub::cachedLastSeen(int64 tgId) const {
	if (const auto it = _profileCache.find(tgId); it != _profileCache.end()) {
		if (it->profile.lastSeen > 0) {
			return it->profile.lastSeen;
		}
	}
	return std::nullopt;
}

bool Hub::IsRecentlySeen(int64 lastSeenUnixSecs) {
	// Heartbeats fire every kHeartbeatIntervalMs; allow one missed beat
	// before considering the user offline, to avoid flicker.
	const auto thresholdSecs = 2 * (kHeartbeatIntervalMs / 1000);
	const auto nowSecs = base::unixtime::now();
	return (lastSeenUnixSecs > 0)
		&& (nowSecs - lastSeenUnixSecs <= thresholdSecs);
}

// ── Admin role management ─────────────────────────────────────────────────────

void Hub::setAdminRole(
		int64 tgId,
		const QString &badge,
		const QString &badgeColor,
		const QString &adminToken,
		Fn<void(bool)> callback) {
	QJsonObject obj;
	obj["badge"]       = badge;
	obj["badge_color"] = badgeColor;
	const auto body = QJsonDocument(obj).toJson(QJsonDocument::Compact);
	putAdmin(u"/admin/role/%1"_q.arg(tgId), body, adminToken, [=](int status) {
		// Invalidate cache so next fetch picks up the new badge.
		_profileCache.remove(tgId);
		if (callback) callback(status == 204);
	});
}

void Hub::removeAdminRole(
		int64 tgId,
		const QString &adminToken,
		Fn<void(bool)> callback) {
	delAdmin(u"/admin/role/%1"_q.arg(tgId), adminToken, [=](int status) {
		_profileCache.remove(tgId);
		if (callback) callback(status == 204);
	});
}

// ── External file detection ───────────────────────────────────────────────────

FileProvider Hub::detectProvider(const QString &url) {
	const auto host = QUrl(url).host().toLower();

	if (host.contains("mega.nz") || host.contains("mega.co.nz")) {
		return FileProvider::Mega;
	}
	if (host.contains("1drv.ms") || host.contains("onedrive.live.com")
			|| host.contains("sharepoint.com")) {
		return FileProvider::OneDrive;
	}
	if (host.contains("drive.google.com") || host.contains("docs.google.com")) {
		return FileProvider::GoogleDrive;
	}
	if (host.contains("dropbox.com") || host.contains("dl.dropboxusercontent.com")) {
		return FileProvider::Dropbox;
	}
	if (host.contains("we.tl") || host.contains("wetransfer.com")) {
		return FileProvider::WeTransfer;
	}
	if (host.contains("gofile.io")) {
		return FileProvider::Gofile;
	}
	if (host.contains("pixeldrain.com")) {
		return FileProvider::Pixeldrain;
	}
	if (host.contains("catbox.moe") || host.contains("litterbox.catbox.moe")) {
		return FileProvider::Catbox;
	}

	// Check if it's a hub relay short link
	const auto &hubUrl = Hub::instance().hubUrl();
	if (!hubUrl.isEmpty() && url.startsWith(hubUrl + "/r/")) {
		return FileProvider::HubRelay;
	}

	// Direct media URL
	const auto path = QUrl(url).path().toLower();
	const auto ext  = path.section('.', -1);
	if (kImageExts.contains(ext) || kVideoExts.contains(ext)
			|| kAudioExts.contains(ext)) {
		return FileProvider::DirectMedia;
	}

	return FileProvider::Unknown;
}

std::optional<ExternalFile> Hub::detectExternalFile(const QString &url) {
	const auto provider = detectProvider(url);
	if (provider == FileProvider::Unknown) {
		return std::nullopt;
	}
	ExternalFile f;
	f.provider = provider;
	f.url      = url;

	// Guess filename from URL path for direct links / catbox
	if (provider == FileProvider::DirectMedia
			|| provider == FileProvider::Catbox) {
		f.filename = QUrl(url).path().section('/', -1);
		const auto ext = f.filename.section('.', -1).toLower();
		if (kImageExts.contains(ext))      f.mime = "image/" + ext;
		else if (kVideoExts.contains(ext)) f.mime = "video/" + ext;
		else if (kAudioExts.contains(ext)) f.mime = "audio/" + ext;
	}

	return f;
}

// ── Bulk user fetch ───────────────────────────────────────────────────────────

void Hub::fetchUsersBatch(
		const QVector<int64> &ids,
		Fn<void(QHash<int64, HubProfile>)> callback) {
	if (!isConfigured() || ids.isEmpty()) {
		if (callback) callback({});
		return;
	}
	QJsonArray arr;
	for (const auto id : ids) arr.append(id);
	QJsonObject bodyObj;
	bodyObj["ids"] = arr;
	const auto body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);
	post(u"/users/batch"_q, body, [=](QJsonDocument doc, int status) {
		QHash<int64, HubProfile> result;
		if (status != 200 || !doc.isObject()) {
			if (callback) callback(result);
			return;
		}
		for (const auto &key : doc.object().keys()) {
			const auto o = doc.object()[key].toObject();
			HubProfile p;
			p.tgId        = o["tg_id"].toVariant().toLongLong();
			p.avatarUrl   = o["avatar_url"].toString();
			p.color       = o["color"].toString();
			p.badge       = o["badge"].toString();
			p.badgeColor  = o["badge_color"].toString();
			p.statusEmoji = o["status_emoji"].toString();
			p.statusText  = o["status_text"].toString();
			_profileCache[p.tgId] = { p, crl::now() };
			result[p.tgId] = p;
		}
		if (callback) callback(result);
	});
}

// ── Heartbeat (presence) ──────────────────────────────────────────────────────

void Hub::sendHeartbeat(int64 myTgId) {
	if (!isConfigured() || myTgId == 0) return;
	_myTgId = myTgId;
	post(u"/heartbeat/%1"_q.arg(myTgId), {}, [](QJsonDocument, int) {});
	if (!_heartbeatTimer) {
		_heartbeatTimer = new QTimer(this);
		_heartbeatTimer->setInterval(kHeartbeatIntervalMs);
		connect(_heartbeatTimer, &QTimer::timeout, this, [this] {
			if (_myTgId != 0 && isConfigured()) {
				post(u"/heartbeat/%1"_q.arg(_myTgId), {}, [](QJsonDocument, int) {});
			}
		});
	}
	if (!_heartbeatTimer->isActive()) {
		_heartbeatTimer->start();
	}
}

// ── Announcements ─────────────────────────────────────────────────────────────

void Hub::fetchAnnouncements(Fn<void(QVector<HubAnnouncement>)> callback) {
	get(u"/announcements"_q, [=](QJsonDocument doc) {
		QVector<HubAnnouncement> result;
		if (!doc.isArray()) {
			if (callback) callback(result);
			return;
		}
		for (const auto &v : doc.array()) {
			const auto o = v.toObject();
			HubAnnouncement a;
			a.id        = o["id"].toString();
			a.title     = o["title"].toString();
			a.body      = o["body"].toString();
			a.pinned    = o["pinned"].toBool();
			a.createdAt = o["created_at"].toVariant().toLongLong();
			result.push_back(a);
		}
		if (callback) callback(result);
	});
}

// ── Hub-pushed theme ──────────────────────────────────────────────────────────

void Hub::fetchHubTheme(Fn<void(std::optional<BgPalette>)> callback) {
	get(u"/theme"_q, [=](QJsonDocument doc) {
		if (!doc.isObject() || !doc.object().contains("bg")) {
			if (callback) callback(std::nullopt);
			return;
		}
		const auto o = doc.object();
		const auto toColor = [](const QString &hex) { return QColor(hex); };
		BgPalette p;
		p.name    = o["name"].toString();
		p.bg      = toColor(o["bg"].toString());
		p.surface = toColor(o["surface"].toString());
		p.card    = toColor(o["card"].toString());
		p.msgIn   = toColor(o["msg_in"].toString());
		p.msgOut  = toColor(o["msg_out"].toString());
		p.accent  = toColor(o["accent"].toString());
		p.text    = toColor(o["text"].toString());
		p.textSub = toColor(o["text_sub"].toString());
		if (!p.bg.isValid() || !p.accent.isValid()) {
			if (callback) callback(std::nullopt);
			return;
		}
		if (callback) callback(p);
	});
}

} // namespace BetterGram
