/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "history/view/media/history_view_bg_external_file.h"

#include "core/click_handler_types.h"
#include "history/history_item.h"
#include "history/view/history_view_element.h"
#include "history/view/history_view_cursor_state.h"
#include "ui/painter.h"
#include "ui/text/format_values.h"
#include "ui/chat/chat_style.h"
#include "styles/style_chat.h"

namespace HistoryView {
namespace {

QString ProviderLabel(BetterGram::FileProvider p) {
	using P = BetterGram::FileProvider;
	switch (p) {
	case P::Mega:        return u"MEGA"_q;
	case P::OneDrive:    return u"OneDrive"_q;
	case P::GoogleDrive: return u"Google Drive"_q;
	case P::Dropbox:     return u"Dropbox"_q;
	case P::WeTransfer:  return u"WeTransfer"_q;
	case P::Gofile:      return u"Gofile"_q;
	case P::Pixeldrain:  return u"Pixeldrain"_q;
	case P::Catbox:      return u"Catbox"_q;
	case P::HubRelay:    return u"Hub Relay"_q;
	default:             return u"External File"_q;
	}
}

} // namespace

BgExternalFile::BgExternalFile(
	not_null<Element*> parent,
	not_null<HistoryItem*> realParent,
	const BetterGram::ExternalFile &file)
: Media(parent)
, _realParent(realParent)
, _file(file)
, _openLink(std::make_shared<UrlClickHandler>(_file.url)) {
	const auto name = _file.filename.isEmpty()
		? ProviderLabel(_file.provider)
		: _file.filename;
	_nameText.setText(st::semiboldTextStyle, name);
}

QString BgExternalFile::providerName() const {
	return ProviderLabel(_file.provider);
}

QString BgExternalFile::statusText() const {
	auto result = providerName();
	if (_file.size > 0) {
		result += u" • "_q + Ui::FormatSizeText(_file.size);
	}
	return result;
}

QSize BgExternalFile::countOptimalSize() {
	const auto &padding = st::msgPadding;
	const auto height = padding.top()
		+ st::semiboldFont->height
		+ st::normalFont->height
		+ padding.bottom();
	return { st::msgMaxWidth, height };
}

QSize BgExternalFile::countCurrentSize(int newWidth) {
	const auto &padding = st::msgPadding;
	const auto height = padding.top()
		+ st::semiboldFont->height
		+ st::normalFont->height
		+ padding.bottom();
	return { newWidth, height };
}

void BgExternalFile::draw(Painter &p, const PaintContext &context) const {
	const auto stm = context.messageStyle();
	const auto &padding = st::msgPadding;
	const auto left = padding.left();
	const auto available = width() - left - padding.right();
	auto top = padding.top();

	p.setFont(st::semiboldFont);
	p.setPen(stm->historyTextFg);
	_nameText.drawElided(p, left, top, available);
	top += st::semiboldFont->height;

	p.setFont(st::normalFont);
	p.setPen(stm->msgDateFg);
	p.drawText(
		left,
		top + st::normalFont->ascent,
		st::normalFont->elided(statusText(), available));
}

TextState BgExternalFile::textState(
		QPoint point,
		StateRequest request) const {
	auto result = TextState(_realParent);
	if (QRect(0, 0, width(), height()).contains(point)) {
		result.link = _openLink;
	}
	return result;
}

} // namespace HistoryView
