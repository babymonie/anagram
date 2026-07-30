/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "history/view/media/history_view_media.h"
#include "bettergram/hub.h"

namespace HistoryView {

class BgExternalFile final : public Media {
public:
	BgExternalFile(
		not_null<Element*> parent,
		not_null<HistoryItem*> realParent,
		const BetterGram::ExternalFile &file);

	void draw(Painter &p, const PaintContext &context) const override;
	TextState textState(
		QPoint point,
		StateRequest request) const override;

	bool toggleSelectionByHandlerClick(
			const ClickHandlerPtr &p) const override {
		return true;
	}
	bool dragItemByHandler(const ClickHandlerPtr &p) const override {
		return true;
	}

	bool needsBubble() const override {
		return true;
	}
	bool customInfoLayout() const override {
		return false;
	}

private:
	QSize countOptimalSize() override;
	QSize countCurrentSize(int newWidth) override;

	[[nodiscard]] QString providerName() const;
	[[nodiscard]] QString statusText() const;

	not_null<HistoryItem*> _realParent;
	BetterGram::ExternalFile _file;
	ClickHandlerPtr _openLink;
	Ui::Text::String _nameText;

};

} // namespace HistoryView
