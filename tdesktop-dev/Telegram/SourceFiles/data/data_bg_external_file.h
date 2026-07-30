/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "data/data_media_types.h"
#include "bettergram/hub.h"

namespace HistoryView {
class BgExternalFile;
} // namespace HistoryView

namespace Data {

class MediaBgExternalFile final : public Media {
public:
	MediaBgExternalFile(
		not_null<HistoryItem*> parent,
		const BetterGram::ExternalFile &file);

	[[nodiscard]] const BetterGram::ExternalFile &externalFile() const;

	std::unique_ptr<Media> clone(not_null<HistoryItem*> parent) override;

	TextWithEntities notificationText() const override;
	QString pinnedTextSubstring() const override;
	TextForMimeData clipboardText() const override;

	bool updateInlineResultMedia(const MTPMessageMedia &media) override;
	bool updateSentMedia(const MTPMessageMedia &media) override;

	std::unique_ptr<HistoryView::Media> createView(
		not_null<HistoryView::Element*> message,
		not_null<HistoryItem*> realParent,
		HistoryView::Element *replacing = nullptr) override;

private:
	BetterGram::ExternalFile _file;

};

} // namespace Data
