# BetterGram

**BetterGram** is a modded Telegram Desktop client — Vencord for Telegram.

It unlocks premium features for free, adds a federated **Hub** system for BetterGram-exclusive extras (animated avatars, role badges, custom reactions, rich external file previews), a floating voice chat mini-player, and supports self-hosted AI/translation backends.

Built on top of [Telegram Desktop][telegram_desktop] (GPLv3). Fully compatible with normal Telegram clients — non-BG users see standard messages.

---

## Table of Contents

1. [What Makes BetterGram Different](#what-makes-bettergram-different)
2. [Feature Overview](#feature-overview)
3. [Setup Guide](#setup-guide)
4. [BetterGram Settings Reference](#bettergram-settings-reference)
5. [Hubs — Full Guide](#hubs--full-guide)
   - [What is a Hub?](#what-is-a-hub)
   - [Joining a Hub](#joining-a-hub)
   - [Setting Up Your BG Profile](#setting-up-your-bg-profile)
   - [Hub Roles & Badges](#hub-roles--badges)
   - [External File Attachments](#external-file-attachments)
   - [BG Reactions](#bg-reactions)
6. [Voice Chat Mini-Player](#voice-chat-mini-player)
7. [Running Your Own Hub](#running-your-own-hub)
8. [Hub Admin Guide](#hub-admin-guide)
9. [Hub API Reference](#hub-api-reference)
10. [Translation Setup](#translation-setup)
11. [Voice-to-Text Setup](#voice-to-text-setup)
12. [AI Compose Setup](#ai-compose-setup)
13. [Building from Source](#building-from-source)
14. [Source Layout](#source-layout)
15. [FAQ](#faq)

---

## What Makes BetterGram Different

| | Telegram Desktop | BetterGram |
|---|---|---|
| Premium features | Requires subscription | All unlocked for free |
| Animated avatars | Premium only | Free via Hub |
| Custom name colours | No | Yes, via Hub |
| Role badges | Admin/Owner only | Hub-assignable custom roles |
| External file links | Plain URLs | Native file bubbles (MEGA, Drive, etc.) |
| Floating voice mini-player | No | Yes |
| Chat translation | Premium | Free via LibreTranslate/Lingva |
| Voice-to-text | Premium | Free via local Whisper |
| Sponsored messages | Always shown | Hideable |
| Self-hosted backend | No | Yes — Hub, Whisper, translation |
| Ghost messages (deleted) | Gone forever | Preserved with "Deleted" overlay |
| Always-on timestamps | Some modes only | Toggle to always show |
| Typing indicator control | Always sends | Per-peer suppression list |
| Character counter | Only when over limit | Always-on N / 4096 label |
| Link tracking params | Passed through | Automatically stripped |
| Notification sounds | Global only | Per-chat custom audio file |
| Contact notes | No | Private per-contact notepad |

---

## Feature Overview

### Premium Unlocks (free for all BG users)
- **Translate entire chats** — powered by any LibreTranslate or Lingva instance
- **Chat folder tags** — colour-labelled folder organisation
- **Set wallpaper for both sides** of a conversation independently
- **Voice-to-text transcription** — powered by local Whisper
- **AI message compose** — via any OpenAI-compatible endpoint (Ollama, LM Studio, etc.)

### Hub Features (BG-to-BG)
- **Animated avatars** — GIF/WebP profile pictures
- **Custom name colours** — your hex colour shown next to your name in chats
- **Custom role badges** — coloured pill labels (Moderator, OG, Staff, etc.) next to names
- **BG Reactions** — emoji reactions stored on the hub, visible to all BG users
- **External file attachments** — MEGA, OneDrive, Google Drive, Dropbox, WeTransfer, Gofile, Pixeldrain, Catbox links render as native file bubbles

### Voice Chat
- **Floating mini-player** — a compact draggable overlay when you're in a voice/group call

### Power Features (Settings → BetterGram → Power Features)
- **Ghost Messages** — when someone deletes a message, BetterGram keeps it locally, shown dimmed with a "🗑 Deleted" overlay. Clears on app restart.
- **Always-show timestamps** — force HH:MM to appear on every message type, including media with custom info layouts.
- **Character counter** — always-visible `N / 4096` label in the compose box corner, replacing the "over limit" label when within the limit.
- **Link de-tracker** — automatically strips UTM, fbclid, gclid, and other tracking parameters from every URL before it opens in a browser.
- **Typing indicator suppressor** — add peer IDs to a list and BetterGram silently skips the "typing…" notification for those contacts.
- **Custom notification sounds** — assign a local audio file (MP3, WAV, etc.) as the notification sound for a specific contact.
- **Contact notes** — right-click any DM contact → Add/Edit Contact Note. Private notepad stored locally.

### Privacy & Cleanup
- Hide sponsored messages (channels + search results)
- Hide stories bar, story badges, story unread indicators
- Hide premium upsells, badges, menu items, and upgrade buttons

### Media Downloads
- Download last 100 / 500 / all media (up to 2000 items) from any chat

---

## Setup Guide

### 1. Download or Build

See [Building from Source](#building-from-source). Pre-built binaries are not yet distributed.

### 2. Open BetterGram Settings

**Settings → BetterGram** (scroll to the bottom of the settings panel).

### 3. Connect to a Hub (optional)

Under **Hub** → **Hub Server URL**, enter the address of a public hub or your own. This enables animated avatars, name colours, role badges, reactions, and external file previews between BG users.

### 4. Set Your BG Profile (optional)

Under **Hub** → fill in your **Animated Avatar URL**, **Name Colour** (hex), and click **Save to Hub**.

### 5. Configure Unlocked Features (optional)

- **Translation** → enter a LibreTranslate or Lingva server URL
- **Voice** → enter a Whisper HTTP API URL and key
- **AI Compose** → enter an OpenAI-compatible URL, key, and model name

---

## BetterGram Settings Reference

All settings live under **Settings → BetterGram**.

### Hub

| Setting | What it does |
|---|---|
| Hub Server URL | Address of the BetterGram Hub to connect to. Leave empty to disable all hub features. |
| Animated Avatar URL | Direct URL to a GIF or WebP for your profile picture (stored on the hub). |
| Name Colour | Hex colour code (`#RRGGBB`) shown next to your name for other BG users. |
| Save to Hub | Push your current BG profile (avatar + colour) to the hub. |

### Hub Roles Admin

Only relevant if you run or administer a hub.

| Setting | What it does |
|---|---|
| Hub Admin Token | The `ADMIN_TOKEN` set on your hub server. Required to assign/remove roles. |
| Assign Role | Opens a dialog: enter a user's Telegram ID, a role badge name, and a hex colour. |
| Remove Role | Opens a dialog: enter a user's Telegram ID to clear their badge. |

### Stories

| Setting | What it does |
|---|---|
| Hide Stories Bar | Remove the stories strip at the top of the chat list. |
| Hide Story Badges | Remove the coloured ring on avatars indicating an unread story. |
| Hide Story Unread Indicators | Remove the dot indicator on stories. |

### Premium

| Setting | What it does |
|---|---|
| Hide Premium Badges | Remove the ⭐ star badge next to subscribers' names. |
| Hide Premium Popups | Suppress "Subscribe to unlock" popups. |
| Hide Premium Menu Items | Remove Gift and other premium-gated menu entries. |
| Hide Upgrade Buttons | Remove "Get Premium" banners and buttons. |
| Hide Sponsored Messages (Ads) | Remove sponsored messages in channels and search results. |

### Translation

| Setting | What it does |
|---|---|
| Translation Server URL | Template URL for LibreTranslate or Lingva. Example: `https://translate.example.com/translate?q={text}&source={from}&target={to}` |

### Voice to Text

| Setting | What it does |
|---|---|
| Whisper API URL | Base URL of your Whisper HTTP server. |
| Whisper API Key | API key if required by your server. |

### AI Compose

| Setting | What it does |
|---|---|
| AI Compose URL | Base URL of any OpenAI-compatible API. |
| AI Compose Key | API key. |
| AI Compose Model | Model name (e.g. `llama3`, `gpt-4o`). |

### Power Features

| Setting | What it does |
|---|---|
| Ghost Messages | Keep remotely-deleted messages visible (dimmed, "🗑 Deleted" label). Session-only. |
| Always show timestamps | Force HH:MM to display on all message types, including media bubbles. |
| Character counter | Show `N / 4096` live in the compose box while typing. |
| Strip tracking params | Strip UTM, fbclid, gclid, and similar trackers from links before opening. |
| Typing Indicator Suppressor | Enter peer IDs — no "typing…" signal is sent to those contacts. |
| Custom Notification Sounds | Enter a peer ID and a path to a local audio file to override the notification sound for that contact. |

---

## Contact Notes

Right-click any DM contact in the chat list → **Add Contact Note** (or **Edit Contact Note** if one exists). Opens a free-text notepad stored locally. Notes persist across sessions (stored in BetterGram's local KV store).

Use this for: nicknames, context reminders, project notes, CRM-style info about a contact.

---

## Hubs — Full Guide

### What is a Hub?

A **Hub** is a lightweight, self-hosted server that adds a community layer on top of Telegram — similar to how Vencord plugin servers or Minecraft servers work.

Key properties:
- **No auth required** — the hub identifies users by their Telegram user ID only
- **Stores only metadata** — never files, never messages, never credentials
- **Federated** — anyone can run one; users choose which hub to join
- **Invisible to non-BG users** — hub features are only visible between BetterGram clients

Think of it like a "community layer" sitting behind your Telegram group. BG users in the same group who are connected to the same hub see each other's animated avatars, role badges, custom reactions, and external file previews. Regular Telegram users see normal messages.

### Joining a Hub

1. Get the URL of a hub (either run by yourself or by someone in your community)
2. **Settings → BetterGram → Hub → Hub Server URL**
3. Paste the URL and press Enter
4. The status will change to **Connected** once the hub is reachable

You can verify it's working by going to **Settings → BetterGram → Hub** — if the status says **Connected**, the hub is live.

### Setting Up Your BG Profile

Your BG profile is what other BG users see when they interact with you in a hub-connected group.

1. **Settings → BetterGram → Hub**
2. Fill in:
   - **Animated Avatar URL** — a direct link to a GIF, WebP, or any image (hosted anywhere publicly accessible, e.g. Imgur)
   - **Name Colour** — a hex colour like `#FF6B6B` (shown next to your name in messages)
3. Click **Save to Hub**

Your profile is stored on the hub and synced to other BG users within about 5 minutes (profile cache TTL).

**Example GIF avatar sources:**
- Upload to `catbox.moe` for a permanent free host
- Use any Imgur GIF direct link (`https://i.imgur.com/xxx.gif`)
- Self-host on your hub's relay system

### Hub Roles & Badges

Roles are coloured pill labels shown inline next to a user's name in chat — like Discord's role colours but built into message bubbles. Only BG users see them.

**Examples of what they look like:**
```
[Adrin] [Moderator]  ← blue pill badge next to the name
[User123] [OG]       ← custom colour badge
[Staff] [Admin]      ← red badge
```

#### Seeing badges

Badges are fetched automatically when BetterGram first loads a message from a known hub user. There is a short delay on first load (hub profile fetch). After that, they are cached for 5 minutes.

#### Assigning badges (hub admin only)

You must be the administrator of the hub (you have the `ADMIN_TOKEN`).

1. **Settings → BetterGram → Hub Roles Admin**
2. Enter your **Hub Admin Token** (the `ADMIN_TOKEN` set on the server)
3. Click **Assign Role**:
   - **User Telegram ID** — the numeric Telegram user ID of the person you want to badge. Right-click their profile → Copy ID (or use `@userinfobot`)
   - **Role Badge** — the text to show, e.g. `Moderator`, `OG`, `Staff`
   - **Badge Color** — hex colour for the pill, e.g. `#5B7FF1` (blue), `#E74C3C` (red)
4. Click **Assign Role**. The badge appears for all BG users in the hub within the next cache refresh (~5 min, or immediately after a restart)

To **remove** a badge: click **Remove Role**, enter the user's Telegram ID.

### External File Attachments

BetterGram renders links from major file hosts as native file bubbles, so BG users see proper file cards with filename, size icon, and download button — instead of a bare URL.

**Supported providers:**

| Service | Notes |
|---|---|
| MEGA | `mega.nz` links |
| OneDrive | `1drv.ms` and `sharepoint.com` links |
| Google Drive | `drive.google.com` links |
| Dropbox | `dropbox.com` and `dl.dropboxusercontent.com` |
| WeTransfer | `we.tl` and `wetransfer.com` |
| Gofile | `gofile.io` links |
| Pixeldrain | `pixeldrain.com` links |
| Catbox | `catbox.moe` and `litterbox.catbox.moe` |
| Hub Relay | Short links from your hub (`/r/:id`) |
| Direct Media | Any URL ending in `.jpg`, `.png`, `.mp4`, `.mp3`, etc. |

**How it works:** BetterGram scans all links in messages. When a known provider URL is found, it injects a native file media bubble inline in the message. Non-BG users see the original link as-is.

**Hub Relay** is BetterGram's own short-link system: register any external URL with the hub and share the short link. All BG users connected to the same hub see it as a native attachment.

### BG Reactions

BG Reactions are emoji reactions stored on the hub, separate from Telegram's built-in reaction system. All BG users connected to the same hub see them.

Currently accessible via the Hub API (client-side UI coming in a future update). Reactions are stored per `(peer_id, message_id)` pair.

---

## Voice Chat Mini-Player

When you're in a group voice call or a video call, BetterGram adds a **⋯** button in the call top bar (next to the hang-up button).

Clicking it opens a compact floating mini-player:

```
╭─────────────────────────────────╮
│  [M]  Group Name       [^]  [x] │
╰─────────────────────────────────╯
```

| Button | Action |
|---|---|
| **M** (left circle) | Toggle mute. Circle turns red when muted, green when active. |
| **^** (right-center circle) | Expand back to the full call panel. |
| **x** (right circle) | Leave the call. |
| Double-click anywhere | Expand to full panel. |
| Click + drag | Move the mini-player anywhere on screen. |

The mini-player stays on top of all other windows. A green glowing ring around it pulses when anyone in the call is speaking.

Click the **⋯** button again to close the mini-player without leaving the call.

---

## Running Your Own Hub

The hub server is a single Go binary with no CGO dependency, using SQLite for storage.

### Quick Start

```bash
cd bettergram-hub
go build -o bettergram-hub .
HUB_URL=https://hub.yourdomain.com ADMIN_TOKEN=mysecrettoken ./bettergram-hub
```

Or with Docker:

```bash
# Create a docker-compose.yml:
services:
  hub:
    build: .
    ports:
      - "8080:8080"
    environment:
      HUB_URL: https://hub.yourdomain.com
      HUB_NAME: My Community Hub
      ADMIN_TOKEN: mysecrettoken
      DB_PATH: /data/hub.db
    volumes:
      - hub-data:/data
volumes:
  hub-data:
```

```bash
docker compose up -d
```

### Environment Variables

| Variable | Default | Description |
|---|---|---|
| `PORT` | `8080` | HTTP port to listen on |
| `DB_PATH` | `./hub.db` | SQLite database file path |
| `HUB_NAME` | `BetterGram Hub` | Display name shown in `/info` |
| `HUB_URL` | `http://localhost:8080` | Public-facing base URL (used to generate relay short links) |
| `ADMIN_TOKEN` | *(empty)* | Secret token required to use admin endpoints. Leave empty to disable admin routes. |

### Reverse Proxy (nginx example)

```nginx
server {
    server_name hub.yourdomain.com;
    location / {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

Run Certbot for HTTPS. BetterGram clients require HTTPS for hub connections in production.

### Sharing Your Hub

Give users your hub URL (e.g. `https://hub.yourdomain.com`) and tell them to enter it in:
**Settings → BetterGram → Hub → Hub Server URL**

Users never need to create an account or log in. Their Telegram user ID is used as the identifier automatically.

---

## Hub Admin Guide

Hub admins can assign role badges to any user connected to the hub.

### Getting Your Admin Token

The admin token is the `ADMIN_TOKEN` you set when starting the hub server. Keep it secret — anyone with this token can modify any user's role on your hub.

### Assigning a Role via BetterGram Client

1. **Settings → BetterGram → Hub Roles Admin**
2. Set your **Hub Admin Token** (stored locally on your machine, not sent to Telegram)
3. Click **Assign Role**:

| Field | Example | Notes |
|---|---|---|
| User Telegram ID | `123456789` | Numeric ID only. Use `@userinfobot` or right-click profile → Copy ID to get it. |
| Role Badge | `Moderator` | Short text shown as the badge. Keep it under ~12 chars. |
| Badge Color | `#5B7FF1` | Hex colour. Blue = `#5B7FF1`, Red = `#E74C3C`, Green = `#2ECC71`. |

4. Click **Assign Role**. Done. The badge will appear for BG users on their next profile cache refresh (~5 min).

### Assigning a Role via API (curl)

```bash
curl -X PUT https://hub.yourdomain.com/admin/role/123456789 \
  -H "Content-Type: application/json" \
  -H "X-Admin-Token: mysecrettoken" \
  -d '{"badge": "Moderator", "badge_color": "#5B7FF1"}'
```

### Removing a Role via API

```bash
curl -X DELETE https://hub.yourdomain.com/admin/role/123456789 \
  -H "X-Admin-Token: mysecrettoken"
```

### Listing All Roles

```bash
curl https://hub.yourdomain.com/admin/roles \
  -H "X-Admin-Token: mysecrettoken"
```

Returns:
```json
[
  {"tg_id": 123456789, "badge": "Moderator", "badge_color": "#5B7FF1"},
  {"tg_id": 987654321, "badge": "OG",        "badge_color": "#E74C3C"}
]
```

### Suggested Role Palette

| Role | Badge Text | Colour |
|---|---|---|
| Owner | `Owner` | `#FFD700` |
| Admin | `Admin` | `#E74C3C` |
| Moderator | `Mod` | `#5B7FF1` |
| Staff | `Staff` | `#9B59B6` |
| OG Member | `OG` | `#E67E22` |
| Verified | `✓` | `#2ECC71` |
| Booster | `Boost` | `#FF69B4` |

---

## Hub API Reference

All endpoints return JSON.

### Public Endpoints

```
GET  /info
```
Returns hub metadata: name, version, features, user count.

```
GET  /users
```
Returns array of all registered Telegram user IDs.

```
GET  /user/:tg_id
```
Returns a user's BG profile:
```json
{
  "tg_id": 123456789,
  "avatar_url": "https://i.imgur.com/xxx.gif",
  "color": "#FF6B6B",
  "badge": "Moderator",
  "badge_color": "#5B7FF1",
  "status_emoji": "🔥",
  "status_text": "Building stuff"
}
```

```
PUT  /user/:tg_id
```
Body: `{ avatar_url, color, status_emoji, status_text }`. Updates the user's own profile fields. Note: `badge` and `badge_color` can only be set by an admin.

```
DELETE /user/:tg_id
```
Remove a user's profile from the hub.

```
POST /relay
```
Body: `{ url, filename, size, mime }`. Registers an external URL as a hub relay short link.

```
GET /relay/:id
```
Fetch relay metadata by ID.

```
GET /r/:id
```
Redirect to the original external URL.

```
GET /reactions/:peer_id/:msg_id
```
Fetch all BG reactions for a message.

```
POST /reactions/:peer_id/:msg_id
```
Body: `{ tg_id, emoji }`. Add or update a reaction.

```
DELETE /reactions/:peer_id/:msg_id/:tg_id
```
Remove a user's reaction.

### Admin Endpoints (require `X-Admin-Token` header)

```
GET    /admin/roles
PUT    /admin/role/:tg_id    body: { badge, badge_color }
DELETE /admin/role/:tg_id
```

---

## Translation Setup

BetterGram uses any LibreTranslate or Lingva instance for chat translation.

### Recommended: Self-hosted LibreTranslate

```bash
docker run -p 5000:5000 libretranslate/libretranslate
```

Then in BetterGram: **Settings → BetterGram → Translation → Translation Server URL**:
```
http://localhost:5000
```

### Public Lingva instance

Enter the base URL of any public Lingva server. BetterGram will construct the translation API call from it.

### Using Translation

Right-click any message → **Translate** (or use the Translate button in the chat header for full-chat translation).

---

## Voice-to-Text Setup

BetterGram delegates transcription to any Whisper-compatible HTTP server.

### Recommended: whisper.cpp server

```bash
git clone https://github.com/ggerganov/whisper.cpp
cd whisper.cpp
make -j server
./server -m models/ggml-base.en.bin
```

Then in BetterGram: **Settings → BetterGram → Voice → Whisper API URL**:
```
http://localhost:8080
```

### Using Voice-to-Text

In any voice message, tap the **transcribe** button (📝 icon below the voice waveform).

---

## AI Compose Setup

BetterGram supports any OpenAI-compatible API for message composition. No specific provider required.

### Ollama (local, free)

```bash
ollama serve
ollama pull llama3
```

Settings:
- **URL**: `http://localhost:11434/v1`
- **Key**: `ollama` (any non-empty string)
- **Model**: `llama3`

### LM Studio

Start LM Studio's local server. Use URL `http://localhost:1234/v1`, any key.

### OpenAI / any cloud provider

Use the provider's API URL, your API key, and the model name.

### Using AI Compose

Click the ✨ (sparkle) button in the compose toolbar, type a prompt or select a custom style.

---

## Building from Source

BetterGram uses the same CMake build system as upstream Telegram Desktop.

### Prerequisites

Follow [Telegram Desktop's upstream build docs][win] to install Qt 6, OpenSSL, and platform toolchain.

### macOS

```bash
git clone --recursive https://github.com/yourfork/bettergram.git
cd bettergram
cmake -B out -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build out --target Telegram
```

### Windows (x64 Native Tools Command Prompt)

```bash
cmake -B out -G Ninja
cmake --build out --config Debug --target Telegram
```

### Linux (Docker)

```bash
Telegram/build/docker/centos_env/build_debug.sh
```

### Building the Hub Server

```bash
cd bettergram-hub
go build -o bettergram-hub .
```

Requires Go 1.22+. No CGO. Cross-compile freely:

```bash
GOOS=linux GOARCH=amd64 go build -o bettergram-hub-linux-amd64 .
```

---

## Source Layout

BetterGram additions on top of upstream Telegram Desktop:

```
Telegram/SourceFiles/
  bettergram/
    hub.h / hub.cpp               Hub singleton, profile cache, HTTP helpers
    bg_call_mini.h / .cpp         Floating voice call mini-player widget

  data/
    data_bg_external_file.h/.cpp  Data::Media subclass for external file links

  history/view/
    history_view_message.cpp      Patched: hub badge pill in paintFromName()
  history/view/media/
    history_view_bg_external_file.h/.cpp  Renders provider links as file bubbles

  settings/sections/
    settings_bettergram.h/.cpp    BetterGram settings page + hub roles admin UI

  calls/
    calls_top_bar.h/.cpp          Patched: adds mini-player toggle button

  core/
    core_settings.h/.cpp          Patched: BG-specific KV preference keys

  history/
    history_item.cpp              Patched: inject external file media from entities

  api/
    api_peer_search.cpp           Patched: skip sponsored search results

  data/components/
    sponsored_messages.cpp        Patched: hide sponsored messages early return

  settings/
    settings_privacy_controllers.cpp  Patched: hide read-time upsell

bettergram-hub/
  main.go                         Hub server (chi router, SQLite, no CGO)
  Dockerfile
  docker-compose.yml
```

### BetterGram KV Preference Keys

Stored in Telegram's local KV store (no new files needed):

| Key | Setting |
|---|---|
| `bettergram-hub-url` | Hub server URL |
| `bg-avatar-url` | Animated avatar URL |
| `bg-name-color` | Name colour hex |
| `bg-hub-admin-token` | Hub admin token (local only) |
| `translate-url-template` | Translation server URL |
| `whisper-api-url` | Whisper server URL |
| `whisper-api-key` | Whisper API key |
| `ai-compose-api-url` | AI compose URL |
| `ai-compose-api-key` | AI compose key |
| `ai-compose-api-model` | AI compose model |
| `hide-stories-bar` | Hide stories strip |
| `hide-story-badges` | Hide story rings |
| `hide-story-notifications` | Hide story dots |
| `hide-premium-badges` | Hide ⭐ badges |
| `hide-premium-upsells` | Hide upgrade popups |
| `hide-premium-menu-items` | Hide gift menu entries |
| `hide-upgrade-buttons` | Hide upgrade banners |
| `hide-sponsored-messages` | Hide sponsored messages |
| `bg-ghost-messages` | Ghost Messages enabled |
| `bg-always-timestamps` | Always-show timestamps enabled |
| `bg-char-counter` | Character counter enabled |
| `bg-link-detracker` | Link de-tracker enabled |
| `bg-typing-suppress` | Comma-separated list of suppressed peer IDs |
| `peer-note-{peerId}` | Per-peer contact note text |
| `peer-sound-{peerId}` | Per-peer notification sound file path |

---

## FAQ

**Q: Will my account get banned?**
BetterGram makes no API calls that violate Telegram's terms of service. It does not automate messages, spam, or modify the Telegram protocol. All BetterGram extras (hub features, badges, reactions) are completely client-side or go to your own hub server — Telegram's servers never see them.

**Q: Can normal Telegram users see BG features?**
No. Hub features (animated avatars, badges, reactions, external file bubbles) are only visible between BetterGram clients connected to the same hub. Normal users see standard messages and plain URLs.

**Q: Do I need to run a hub?**
No. You can use any public hub, or use BetterGram with no hub at all — you still get all the premium unlocks, mini-player, translation, voice-to-text, and AI compose.

**Q: Can I use multiple hubs?**
Not simultaneously — BetterGram connects to one hub at a time. Switch by changing the Hub Server URL in settings. Community hubs may federate with each other in a future version.

**Q: Is the hub server secure?**
The hub stores only metadata: Telegram user IDs, avatar URLs you provide, name colours, and badge assignments. It stores no messages, no credentials, and no Telegram tokens. There is no authentication — any BG user can write their own profile. Role badges can only be modified by whoever holds the `ADMIN_TOKEN`.

**Q: The hub badge doesn't appear immediately — is that normal?**
Yes. Badges are fetched asynchronously on first load and cached for 5 minutes. On the very first load of a message from a BG user you haven't seen before, there is a short delay. After the first fetch, it appears instantly until the cache expires.

**Q: How do I find a public hub to join?**
Ask in a BetterGram community. There is no central hub registry yet — hubs are distributed by word of mouth, like Minecraft servers.

**Q: The mini-player buttons show letters (M, ^, x) instead of icons — is that intended?**
Currently yes — proper icons are pending. The letters are functional: M = mute, ^ = expand, x = leave.

---

## License

GPLv3 with OpenSSL exception — same as upstream Telegram Desktop. See [LICENSE][license].

All BetterGram additions and modifications are also GPLv3.

[telegram_desktop]: https://desktop.telegram.org
[win]: docs/building-win.md
[license]: LICENSE
