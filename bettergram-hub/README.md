# BetterGram Hub

Lightweight community server for [BetterGram](../tdesktop-dev/) — adds animated avatars, role badges, BG-only reactions, and external file relay on top of Telegram groups.

## Quick Start

```bash
go build -o bettergram-hub .
ADMIN_TOKEN=mysecrettoken HUB_URL=https://hub.example.com ./bettergram-hub
```

## Docker

```bash
docker compose up -d
```

## Environment Variables

| Variable | Default | Description |
|---|---|---|
| `PORT` | `8080` | HTTP listen port |
| `DB_PATH` | `./hub.db` | SQLite database path |
| `HUB_NAME` | `BetterGram Hub` | Name shown in `/info` |
| `HUB_URL` | `http://localhost:8080` | Public base URL (for relay short links) |
| `ADMIN_TOKEN` | *(empty)* | Secret token for admin role endpoints. Leave empty to disable. |

## API

See the [full Hub API reference](../tdesktop-dev/README.md#hub-api-reference) in the main README.

### Admin endpoints (need `X-Admin-Token` header)

```bash
# Assign a role badge
curl -X PUT $HUB_URL/admin/role/123456789 \
  -H "X-Admin-Token: $ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"badge": "Moderator", "badge_color": "#5B7FF1"}'

# Remove a role
curl -X DELETE $HUB_URL/admin/role/123456789 \
  -H "X-Admin-Token: $ADMIN_TOKEN"

# List all assigned roles
curl $HUB_URL/admin/roles -H "X-Admin-Token: $ADMIN_TOKEN"
```
