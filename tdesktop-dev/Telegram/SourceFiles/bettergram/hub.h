#pragma once

#include "base/basic_types.h"
#include "bettergram/bg_theme.h"
#include <QtCore/QHash>
#include <QtCore/QJsonObject>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtNetwork/QNetworkAccessManager>
#include <optional>

namespace BetterGram {

struct HubProfile {
	int64 tgId = 0;
	QString avatarUrl;
	QString color;
	QString badge;
	QString badgeColor; // hex colour for the badge pill, e.g. "#5B7FF1"
	QString statusEmoji;
	QString statusText;
	int64 lastSeen = 0; // unix seconds, from the hub's heartbeat table
};

struct HubRelay {
	QString id;
	QString url;
	QString filename;
	int64 size = 0;
	QString mime;
	QString shortUrl;

	[[nodiscard]] bool isImage() const;
	[[nodiscard]] bool isVideo() const;
	[[nodiscard]] bool isAudio() const;
};

struct HubReaction {
	int64 tgId = 0;
	QString emoji;
	int64 ts = 0;
};

struct HubAnnouncement {
	QString id;
	QString title;
	QString body;
	bool pinned = false;
	int64 createdAt = 0;
};

// Supported external file providers — BG renders their links natively.
enum class FileProvider {
	Unknown,
	Mega,
	OneDrive,
	GoogleDrive,
	Dropbox,
	WeTransfer,
	Gofile,
	Pixeldrain,
	Catbox,
	DirectMedia, // direct URL ending in a media extension
	HubRelay,   // hub /r/:id short link
};

struct ExternalFile {
	FileProvider provider = FileProvider::Unknown;
	QString url;
	QString filename;
	QString mime;
	int64 size = 0;
};

class Hub final : public QObject {
	Q_OBJECT
public:
	static Hub &instance();

	// Hub URL management
	void setHubUrl(const QString &url);
	[[nodiscard]] QString hubUrl() const;
	[[nodiscard]] bool isConfigured() const;

	// User profiles
	void fetchProfile(int64 tgId, Fn<void(std::optional<HubProfile>)> callback);
	void fetchUsersBatch(const QVector<int64> &ids, Fn<void(QHash<int64, HubProfile>)> callback);
	void setMyProfile(const HubProfile &profile, Fn<void(bool ok)> callback = nullptr);
	[[nodiscard]] bool isKnownBgUser(int64 tgId) const;
	void refreshUserList();

	// Presence — call once after session starts; repeats on a 5-minute timer.
	void sendHeartbeat(int64 myTgId);

	// Hub v2: announcements and hub-pushed theme
	void fetchAnnouncements(Fn<void(QVector<HubAnnouncement>)> callback);
	void fetchHubTheme(Fn<void(std::optional<BgPalette>)> callback);

	// Synchronous cache read — returns badge text/color if the profile is already
	// in cache, otherwise returns nullopt (caller should call fetchProfile).
	[[nodiscard]] std::optional<std::pair<QString, QString>> cachedBadge(int64 tgId) const;

	// Synchronous cache read — returns the last-seen unix timestamp (seconds)
	// if the profile is already in cache, otherwise nullopt.
	[[nodiscard]] std::optional<int64> cachedLastSeen(int64 tgId) const;

	// True if the cached last-seen timestamp is recent enough to show online.
	[[nodiscard]] static bool IsRecentlySeen(int64 lastSeenUnixSecs);

	// Admin role management (requires the hub's ADMIN_TOKEN).
	void setAdminRole(
		int64 tgId,
		const QString &badge,
		const QString &badgeColor,
		const QString &adminToken,
		Fn<void(bool ok)> callback);
	void removeAdminRole(
		int64 tgId,
		const QString &adminToken,
		Fn<void(bool ok)> callback);

	// Relay (register external URL, get short link)
	void registerRelay(
		const QString &externalUrl,
		const QString &filename,
		int64 size,
		const QString &mime,
		Fn<void(std::optional<HubRelay>)> callback);
	void resolveRelay(
		const QString &id,
		Fn<void(std::optional<HubRelay>)> callback);

	// Reactions
	void addReaction(int64 peerId, int32 msgId, int64 myTgId, const QString &emoji);
	void removeReaction(int64 peerId, int32 msgId, int64 myTgId);
	void fetchReactions(
		int64 peerId,
		int32 msgId,
		Fn<void(QVector<HubReaction>)> callback);

	// Static URL detection
	[[nodiscard]] static std::optional<ExternalFile> detectExternalFile(
		const QString &url);
	[[nodiscard]] static FileProvider detectProvider(const QString &url);

private:
	explicit Hub(QObject *parent = nullptr);
	~Hub() = default;

	void get(const QString &path, Fn<void(QJsonDocument)> callback);
	void post(const QString &path, const QByteArray &body,
		Fn<void(QJsonDocument, int status)> callback);
	void put(const QString &path, const QByteArray &body,
		Fn<void(QJsonDocument, int status)> callback);
	void putAdmin(const QString &path, const QByteArray &body,
		const QString &token,
		Fn<void(int status)> callback);
	void delAdmin(const QString &path,
		const QString &token,
		Fn<void(int status)> callback);
	void del(const QString &path, Fn<void(int status)> callback = nullptr);

	struct CachedProfile {
		HubProfile profile;
		crl::time fetchTime = 0;
	};

	QNetworkAccessManager *_net = nullptr;
	QString _hubUrl;
	QHash<int64, CachedProfile> _profileCache;
	QSet<int64> _knownUsers;
	bool _userListFresh = false;
	int64 _myTgId = 0;
	QTimer *_heartbeatTimer = nullptr;

	static constexpr crl::time kProfileCacheTtl = 5 * 60 * 1000;
};

} // namespace BetterGram
