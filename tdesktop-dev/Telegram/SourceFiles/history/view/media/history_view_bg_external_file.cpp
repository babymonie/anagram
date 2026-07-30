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
#include "history/view/media/history_view_media_common.h"
#include "ui/chat/chat_style.h"
#include "ui/painter.h"
#include "ui/text/format_values.h"
#include "ui/text/text_options.h"
#include "styles/style_chat.h"

namespace HistoryView {

BgExternalFile::BgExternalFile(
	not_null<Element*> parent,
	not_null<HistoryItem*> realParent,
	const BetterGram::ExternalFile &file)
: Media(parent)
, _realParent(realParent)
, _file(file)
, _openLink(std::make_shared<UrlClickHandler>(_file.url, true)) {
	_nameText.setText(
		st::semiboldTextStyle,
		_file.filename.isEmpty() ? providerName() : _file.filename,
		Ui::NameTextOptions());
}

QString BgExternalFile::providerName() const {
	using P = BetterGram::FileProvider;
	switch (_file.provider) {
	case P::Mega: return u"MEGA"_q;
	case P::OneDrive: return u"OneDrive"_q;
	case P::GoogleDrive: return u"Google Drive"_q;
	case P::Dropbox: return u"Dropbox"_q;
	case P::WeTransfer: return u"WeTransfer"_q;
	case P::Gofile: return u"Gofile"_q;
	case P::Pixeldrain: return u"Pixeldrain"_q;
	case P::Catbox: return u"Catbox"_q;
	case P::DirectMedia: return u"Direct"_q;
	case P::HubRelay: return u"BG Hub"_q;
	case P::Unknown: return u"Link"_q;
	}
	return u"Link"_q;
}

QString BgExternalFile::statusText() const {
	const auto pname = providerName();
	if (_file.size > 0) {
		return Ui::FormatSizeText(_file.size) + u" · "_q + pname;
	}
	return pname;
}

QSize BgExternalFile::countOptimalSize() {
	const auto &st = st::msgFileLayout;
	const auto tleft = st.padding.left() + st.thumbSize + st.thumbSkip;
	const auto tright = st.padding.right();

	auto maxWidth = st::msgFileMinWidth;
	accumulate_max(maxWidth, tleft + _nameText.maxWidth() + tright);
	accumulate_min(maxWidth, st::msgMaxWidth);

	auto minHeight = st.padding.top() + st.thumbSize + st.padding.bottom();
	if (!isBubbleTop()) {
		minHeight -= st::msgFileTopMinus;
	}
	return { maxWidth, minHeight };
}

QSize BgExternalFile::countCurrentSize(int newWidth) {
	const auto &st = st::msgFileLayout;
	auto minHeight = st.padding.top() + st.thumbSize + st.padding.bottom();
	if (!isBubbleTop()) {
		minHeight -= st::msgFileTopMinus;
	}
	return { newWidth, minHeight };
}

void BgExternalFile::draw(Painter &p, const PaintContext &context) const {
	const auto stm = context.messageStyle();
	const auto &st = st::msgFileLayout;

	const auto topMinus = isBubbleTop() ? 0 : st::msgFileTopMinus;
	const auto nameleft = st.padding.left() + st.thumbSize + st.thumbSkip;
	const auto nametop = st.nameTop - topMinus;
	const auto nameright = st.padding.right();
	const auto statustop = st.statusTop - topMinus;
	const auto namewidth = width() - nameleft - nameright;

	const auto rthumb = style::rtlrect(
		st.padding.left(),
		st.padding.top() - topMinus,
		st.thumbSize,
		st.thumbSize,
		width());
	const auto innerSize = st::msgFileLayout.thumbSize;
	const auto inner = QRect(
		rthumb.x() + (rthumb.width() - innerSize) / 2,
		rthumb.y() + (rthumb.height() - innerSize) / 2,
		innerSize,
		innerSize);

	p.setPen(Qt::NoPen);
	{
		auto hq = PainterHighQualityEnabler(p);
		p.setBrush(stm->msgFileBg);
		p.drawEllipse(inner);
	}

	const auto initial = providerName().left(1).toUpper();
	p.setPen(stm->historyFileNameFg);
	p.setFont(st::semiboldFont);
	p.drawText(inner, initial, QTextOption(Qt::AlignCenter));

	p.setPen(stm->historyFileNameFg);
	_nameText.draw(p, {
		.position = QPoint(nameleft, nametop),
		.outerWidth = width(),
		.availableWidth = namewidth,
		.elisionLines = 1,
		.elisionMiddle = true,
	});

	p.setFont(st::normalFont);
	p.setPen(stm->mediaFg);
	p.drawTextLeft(nameleft, statustop, width(), statusText());
}

TextState BgExternalFile::textState(
		QPoint point,
		StateRequest request) const {
	auto result = TextState(_parent);
	if (QRect(0, 0, width(), height()).contains(point)) {
		result.link = _openLink;
	}
	return result;
}

} // namespace HistoryView
