/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "data/data_bg_external_file.h"

#include "history/view/media/history_view_bg_external_file.h"

namespace Data {

MediaBgExternalFile::MediaBgExternalFile(
	not_null<HistoryItem*> parent,
	const BetterGram::ExternalFile &file)
: Media(parent)
, _file(file) {
}

const BetterGram::ExternalFile &MediaBgExternalFile::externalFile() const {
	return _file;
}

std::unique_ptr<Media> MediaBgExternalFile::clone(
		not_null<HistoryItem*> parent) {
	return std::make_unique<MediaBgExternalFile>(parent, _file);
}

TextWithEntities MediaBgExternalFile::notificationText() const {
	return { .text = _file.filename };
}

QString MediaBgExternalFile::pinnedTextSubstring() const {
	return _file.filename;
}

TextForMimeData MediaBgExternalFile::clipboardText() const {
	return TextForMimeData::Simple(_file.url);
}

bool MediaBgExternalFile::updateInlineResultMedia(
		const MTPMessageMedia &media) {
	return false;
}

bool MediaBgExternalFile::updateSentMedia(const MTPMessageMedia &media) {
	return false;
}

std::unique_ptr<HistoryView::Media> MediaBgExternalFile::createView(
		not_null<HistoryView::Element*> message,
		not_null<HistoryItem*> realParent,
		HistoryView::Element *replacing) {
	return std::make_unique<HistoryView::BgExternalFile>(
		message,
		realParent,
		_file);
}

} // namespace Data
