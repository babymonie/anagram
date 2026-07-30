/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "bettergram/bg_theme.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "window/themes/window_theme.h"

namespace BetterGram {
namespace {

// Encode as #RRGGBB
[[nodiscard]] QString rgb(const QColor &c) {
	return QStringLiteral("#%1%2%3")
		.arg(c.red(),   2, 16, QChar('0'))
		.arg(c.green(), 2, 16, QChar('0'))
		.arg(c.blue(),  2, 16, QChar('0'));
}

// Encode as #RRGGBBAA
[[nodiscard]] QString rgba(const QColor &c, int a) {
	return QStringLiteral("#%1%2%3%4")
		.arg(c.red(),   2, 16, QChar('0'))
		.arg(c.green(), 2, 16, QChar('0'))
		.arg(c.blue(),  2, 16, QChar('0'))
		.arg(a,         2, 16, QChar('0'));
}

// Lighten in HSL space by pct percentage points
[[nodiscard]] QColor li(const QColor &c, int pct) {
	const auto h = c.toHsl();
	return QColor::fromHslF(
		qMax(0.0, h.hslHueF()),
		h.hslSaturationF(),
		qMin(1.0, h.lightnessF() + pct / 100.0));
}

// Darken in HSL space by pct percentage points
[[nodiscard]] QColor da(const QColor &c, int pct) {
	const auto h = c.toHsl();
	return QColor::fromHslF(
		qMax(0.0, h.hslHueF()),
		h.hslSaturationF(),
		qMax(0.0, h.lightnessF() - pct / 100.0));
}

} // namespace

const QVector<BgPalette> &BundledThemes() {
	static const QVector<BgPalette> kThemes = {
		// 1. Anagram Dark — default, night + BG accent
		{
			u"Anagram Dark"_q,
			QColor(0x17, 0x21, 0x2b), // bg
			QColor(0x0e, 0x16, 0x21), // surface
			QColor(0x17, 0x21, 0x2b), // card
			QColor(0x18, 0x25, 0x33), // msgIn
			QColor(0x2b, 0x52, 0x78), // msgOut
			QColor(0x2a, 0xab, 0xee), // accent
			QColor(0xf5, 0xf5, 0xf5), // text
			QColor(0x70, 0x84, 0x99), // textSub
		},
		// 2. AMOLED Black
		{
			u"AMOLED Black"_q,
			QColor(0x00, 0x00, 0x00), // bg
			QColor(0x0a, 0x0a, 0x0a), // surface
			QColor(0x11, 0x11, 0x11), // card
			QColor(0x0d, 0x11, 0x17), // msgIn
			QColor(0x1a, 0x1a, 0x2e), // msgOut
			QColor(0x2a, 0xab, 0xee), // accent
			QColor(0xf5, 0xf5, 0xf5), // text
			QColor(0x60, 0x68, 0x80), // textSub
		},
		// 3. Nord
		{
			u"Nord"_q,
			QColor(0x2e, 0x34, 0x40), // bg — Polar Night 1
			QColor(0x24, 0x29, 0x33), // surface — darker
			QColor(0x2e, 0x34, 0x40), // card
			QColor(0x3b, 0x42, 0x52), // msgIn — Polar Night 2
			QColor(0x43, 0x4c, 0x5e), // msgOut — Polar Night 3
			QColor(0x88, 0xc0, 0xd0), // accent — Frost 1
			QColor(0xec, 0xef, 0xf4), // text — Snow Storm 3
			QColor(0x7d, 0x8f, 0xa7), // textSub
		},
		// 4. Catppuccin Mocha
		{
			u"Catppuccin Mocha"_q,
			QColor(0x1e, 0x1e, 0x2e), // bg — Base
			QColor(0x18, 0x18, 0x25), // surface — Mantle
			QColor(0x1e, 0x1e, 0x2e), // card
			QColor(0x31, 0x32, 0x44), // msgIn — Surface 0
			QColor(0x45, 0x47, 0x5a), // msgOut — Surface 2
			QColor(0x89, 0xb4, 0xfa), // accent — Blue
			QColor(0xcd, 0xd6, 0xf4), // text
			QColor(0x6c, 0x70, 0x86), // textSub — Overlay 0
		},
		// 5. Gruvbox Dark
		{
			u"Gruvbox Dark"_q,
			QColor(0x28, 0x28, 0x28), // bg — bg0
			QColor(0x1d, 0x20, 0x21), // surface — bg0_h
			QColor(0x28, 0x28, 0x28), // card
			QColor(0x3c, 0x38, 0x36), // msgIn — bg1
			QColor(0x50, 0x49, 0x45), // msgOut — bg3
			QColor(0xfa, 0xbd, 0x2f), // accent — yellow
			QColor(0xeb, 0xdb, 0xb2), // text — fg
			QColor(0x92, 0x83, 0x74), // textSub — gray
		},
	};
	return kThemes;
}

QByteArray DerivePalette(const BgPalette &p) {
	QString text;
	text.reserve(32000);

	// Pre-derive common variants
	const QColor bgActive  = da(p.accent, 30); // dark accent for active dialog row
	const QColor bgActDark = da(p.accent, 35); // even darker for badge text on active
	const QColor accentLi  = li(p.accent, 15); // lighter accent for links/text

	// Append one palette line
	const auto L = [&text](const char *key, const QString &val) {
		text += QLatin1String(key);
		text += QLatin1String(": ");
		text += val;
		text += QLatin1String(";\n");
	};

	// ── Window base ──────────────────────────────────────────────────
	L("windowBg",               rgb(p.bg));
	L("windowFg",               rgb(p.text));
	L("windowBgOver",           rgb(li(p.bg, 5)));
	L("windowBgRipple",         rgb(li(p.bg, 8)));
	L("windowFgOver",           rgb(p.text));
	L("windowSubTextFg",        rgb(p.textSub));
	L("windowSubTextFgOver",    rgb(li(p.textSub, 10)));
	L("windowBoldFg",           rgb(p.text));
	L("windowBoldFgOver",       rgb(p.text));
	L("windowBgActive",         rgb(p.accent));
	L("windowFgActive",         "#ffffff");
	L("windowActiveTextFg",     rgb(accentLi));
	L("windowShadowFg",         "#000000");
	L("windowShadowFgFallback", rgb(p.bg));
	L("shadowFg",               rgba(QColor(0,0,0), 0x30));
	L("slideFadeOutBg",         rgba(QColor(0,0,0), 0x3c));
	L("slideFadeOutShadowFg",   "#000000");
	L("imageBg",                "#000000");
	L("imageBgTransparent",     "#ffffff");
	// ── Active buttons ───────────────────────────────────────────────
	L("activeButtonBg",               rgb(da(p.accent, 15)));
	L("activeButtonBgOver",           rgb(da(p.accent, 8)));
	L("activeButtonBgRipple",         rgb(p.accent));
	L("activeButtonFg",               "#ffffff");
	L("activeButtonFgOver",           "#ffffff");
	L("activeButtonSecondaryFg",      rgb(li(p.accent, 30)));
	L("activeButtonSecondaryFgOver",  rgb(li(p.accent, 30)));
	L("activeLineFg",                 rgb(p.accent));
	L("activeLineFgError",            "#ef5959");
	// ── Light buttons ────────────────────────────────────────────────
	L("lightButtonBg",          rgb(p.card));
	L("lightButtonBgOver",      rgb(li(p.card, 5)));
	L("lightButtonBgRipple",    rgb(li(p.card, 8)));
	L("lightButtonFg",          rgb(p.accent));
	L("lightButtonFgOver",      rgb(p.accent));
	// ── Attention buttons ────────────────────────────────────────────
	L("attentionButtonFg",          "#ec3942");
	L("attentionButtonFgOver",      "#ec3942");
	L("attentionButtonBgOver",      "#592a2a64");
	L("attentionButtonBgRipple",    "#68323264");
	// ── Outline buttons ──────────────────────────────────────────────
	L("outlineButtonBg",        rgb(p.bg));
	L("outlineButtonBgOver",    rgb(li(p.bg, 8)));
	L("outlineButtonOutlineFg", rgb(p.accent));
	L("outlineButtonBgRipple",  rgb(li(p.bg, 12)));
	// ── Menu / popup ─────────────────────────────────────────────────
	L("menuBg",             rgb(p.card));
	L("menuBgOver",         "#ffffff");
	L("menuBgRipple",       rgb(li(p.card, 8)));
	L("menuIconFg",         rgb(p.textSub));
	L("menuIconFgOver",     "#dcdcdc");
	L("menuSubmenuArrowFg", rgb(p.textSub));
	L("menuFgDisabled",     rgb(da(p.textSub, 20)));
	L("menuSeparatorFg",    rgb(li(p.surface, 5)));
	// ── Scroll bars ──────────────────────────────────────────────────
	L("scrollBarBg",     rgba(QColor(0xff,0xff,0xff), 0x53));
	L("scrollBarBgOver", rgba(QColor(0xff,0xff,0xff), 0x7a));
	L("scrollBg",        rgba(QColor(0xff,0xff,0xff), 0x1a));
	L("scrollBgOver",    rgba(QColor(0xff,0xff,0xff), 0x2c));
	// ── Close / cancel icons ─────────────────────────────────────────
	L("smallCloseIconFg",     rgb(p.textSub));
	L("smallCloseIconFgOver", "#a3a3a3");
	L("radialFg",             "#ffffff");
	L("radialBg",             rgba(QColor(0,0,0), 0x56));
	// ── Input fields ─────────────────────────────────────────────────
	L("placeholderFg",         rgb(p.textSub));
	L("placeholderFgActive",   rgb(da(p.textSub, 15)));
	L("inputBorderFg",         rgb(li(p.surface, 8)));
	L("filterInputBorderFg",   rgb(li(p.surface, 10)));
	L("filterInputInactiveBg", rgb(li(p.surface, 5)));
	L("filterInputActiveBg",   rgb(li(p.surface, 10)));
	// ── Checkbox / slider / tooltip ──────────────────────────────────
	L("checkboxFg",      rgb(p.textSub));
	L("sliderBgInactive",rgb(li(p.surface, 15)));
	L("sliderBgActive",  rgb(p.accent));
	L("tooltipBg",       rgb(da(p.surface, 5)));
	L("tooltipFg",       rgb(p.text));
	L("tooltipBorderFg", rgb(da(p.surface, 5)));
	// ── Title bar ────────────────────────────────────────────────────
	L("titleShadow",               "#00000000");
	L("titleBg",                   rgb(p.surface));
	L("titleBgActive",             rgb(li(p.surface, 5)));
	L("titleButtonBg",             rgb(p.surface));
	L("titleButtonFg",             rgb(p.textSub));
	L("titleButtonBgOver",         rgb(li(p.surface, 8)));
	L("titleButtonFgOver",         "#e0e0e0");
	L("titleButtonBgActive",       rgb(li(p.surface, 5)));
	L("titleButtonFgActive",       rgb(p.textSub));
	L("titleButtonBgActiveOver",   rgb(li(p.surface, 12)));
	L("titleButtonFgActiveOver",   "#e0e0e0");
	L("titleButtonCloseBg",            rgb(p.surface));
	L("titleButtonCloseFg",            rgb(p.textSub));
	L("titleButtonCloseBgOver",        "#e92539");
	L("titleButtonCloseFgOver",        "#ffffff");
	L("titleButtonCloseBgActive",      rgb(li(p.surface, 5)));
	L("titleButtonCloseFgActive",      rgb(p.textSub));
	L("titleButtonCloseBgActiveOver",  "#e92539");
	L("titleButtonCloseFgActiveOver",  "#ffffff");
	L("titleFg",       rgb(da(p.textSub, 10)));
	L("titleFgActive", rgb(p.textSub));
	// ── Tray ─────────────────────────────────────────────────────────
	L("trayCounterBg",          "#f23c34");
	L("trayCounterBgMute",      "#888888");
	L("trayCounterFg",          "#ffffff");
	L("trayCounterBgMacInvert", "#ffffff");
	L("trayCounterFgMacInvert", "#ffffff01");
	// ── Layer overlay ────────────────────────────────────────────────
	L("layerBg",        rgba(QColor(0,0,0), 0x7f));
	// ── Cancel icons ─────────────────────────────────────────────────
	L("cancelIconFg",     rgb(p.textSub));
	L("cancelIconFgOver", "#dcdcdc");
	// ── Box ──────────────────────────────────────────────────────────
	L("boxBg",               rgb(p.card));
	L("boxTextFg",           rgb(p.text));
	L("boxTextFgGood",       rgb(p.accent));
	L("boxTextFgError",      "#dc3d3d");
	L("boxTitleFg",          rgb(p.text));
	L("boxSearchBg",         rgb(p.card));
	L("boxTitleAdditionalFg",rgb(p.textSub));
	L("boxTitleCloseFg",     rgb(p.textSub));
	L("boxTitleCloseFgOver", "#dcdcdc");
	L("membersAboutLimitFg", rgb(p.textSub));
	L("contactsBg",          rgb(p.card));
	L("contactsBgOver",      rgb(li(p.card, 8)));
	L("contactsNameFg",      rgb(p.text));
	L("contactsStatusFg",    rgb(p.textSub));
	L("contactsStatusFgOver",rgb(p.textSub));
	L("contactsStatusFgOnline", rgb(li(p.accent, 10)));
	// ── Photo crop ───────────────────────────────────────────────────
	L("photoCropFadeBg",  rgba(QColor(0,0,0), 0x7f));
	L("photoCropPointFg", rgba(QColor(0xff,0xff,0xff), 0x7f));
	// ── Call arrows (calls list) ─────────────────────────────────────
	L("callArrowFg",       rgb(p.accent));
	L("callArrowMissedFg", "#ed5050");
	// ── Login / intro ────────────────────────────────────────────────
	L("introBg",              rgb(p.bg));
	L("introTitleFg",         rgb(p.text));
	L("introDescriptionFg",   rgb(p.textSub));
	L("introErrorFg",         "#e94040");
	L("introCoverTopBg",      rgb(da(p.accent, 30)));
	L("introCoverBottomBg",   rgb(da(p.accent, 20)));
	L("introCoverIconsFg",    rgb(da(p.accent, 10)));
	L("introCoverPlaneTrace", rgb(da(p.accent, 25)));
	L("introCoverPlaneInner", "#ced9e2");
	L("introCoverPlaneOuter", "#97a9b5");
	L("introCoverPlaneTop",   "#ffffff");
	// ── Dialogs list ─────────────────────────────────────────────────
	L("dialogsMenuIconFg",         rgb(p.textSub));
	L("dialogsMenuIconFgOver",     "#dcdcdc");
	L("dialogsBg",                 rgb(p.surface));
	L("dialogsNameFg",             rgb(p.text));
	L("dialogsChatIconFg",         rgb(p.text));
	L("dialogsDateFg",             rgb(p.textSub));
	L("dialogsTextFg",             rgb(p.textSub));
	L("dialogsTextFgService",      rgb(li(p.accent, 20)));
	L("dialogsDraftFg",            "#ff525d");
	L("dialogsVerifiedIconBg",     rgb(li(p.accent, 15)));
	L("dialogsVerifiedIconFg",     rgb(p.surface));
	L("dialogsSendingIconFg",      rgb(da(p.textSub, 10)));
	L("dialogsSentIconFg",         rgb(li(p.accent, 20)));
	L("dialogsUnreadBg",           rgb(p.accent));
	L("dialogsUnreadBgMuted",      rgb(li(p.surface, 15)));
	L("dialogsUnreadFg",           "#ffffff");
	L("dialogsBgOver",             rgb(li(p.surface, 5)));
	L("dialogsOnlineBadgeFg",      rgb(li(p.accent, 10)));
	L("dialogsNameFgOver",         rgb(p.text));
	L("dialogsChatIconFgOver",     rgb(p.text));
	L("dialogsDateFgOver",         rgb(p.textSub));
	L("dialogsTextFgOver",         rgb(li(p.textSub, 15)));
	L("dialogsTextFgServiceOver",  rgb(li(p.accent, 20)));
	L("dialogsDraftFgOver",        "#ff525d");
	L("dialogsVerifiedIconBgOver", rgb(li(p.accent, 15)));
	L("dialogsVerifiedIconFgOver", rgb(p.surface));
	L("dialogsSendingIconFgOver",  rgb(da(p.textSub, 10)));
	L("dialogsSentIconFgOver",     rgb(li(p.accent, 20)));
	L("dialogsUnreadBgOver",       rgb(p.accent));
	L("dialogsUnreadBgMutedOver",  rgb(li(p.surface, 20)));
	L("dialogsUnreadFgOver",       "#ffffff");
	L("dialogsBgActive",           rgb(bgActive));
	L("dialogsNameFgActive",       "#ffffff");
	L("dialogsChatIconFgActive",   "#ffffff");
	L("dialogsDateFgActive",       "#ffffff");
	L("dialogsTextFgActive",       "#ffffff");
	L("dialogsTextFgServiceActive","#ffffff");
	L("dialogsDraftFgActive",      rgb(li(p.accent, 40)));
	L("dialogsVerifiedIconBgActive","#ffffff");
	L("dialogsVerifiedIconFgActive",rgb(bgActive));
	L("dialogsSendingIconFgActive",rgba(QColor(0xff,0xff,0xff), 0x99));
	L("dialogsSentIconFgActive",   "#ffffff");
	L("dialogsUnreadBgActive",     "#ffffff");
	L("dialogsUnreadBgMutedActive",rgb(li(p.accent, 30)));
	L("dialogsUnreadFgActive",     rgb(bgActDark));
	L("dialogsOnlineBadgeFgActive","#ffffff");
	L("dialogsRippleBg",           rgb(li(p.surface, 8)));
	L("dialogsRippleBgActive",     rgb(da(p.accent, 30)));
	L("dialogsForwardBg",          rgb(bgActive));
	L("dialogsForwardFg",          "#ffffff");
	// ── Search bar ───────────────────────────────────────────────────
	L("searchedBarBg",  rgb(li(p.surface, 3)));
	L("searchedBarFg",  rgb(p.textSub));
	// ── Top bar ──────────────────────────────────────────────────────
	L("topBarBg", rgb(p.card));
	// ── Emoji / sticker panel ────────────────────────────────────────
	L("emojiPanBg",         rgb(p.bg));
	L("emojiPanCategories", rgb(p.card));
	L("emojiPanHeaderFg",   rgb(p.textSub));
	L("emojiPanHeaderBg",   rgba(QColor(0xff,0xff,0xff), 0xf2));
	L("stickerPanDeleteBg", rgba(QColor(0,0,0), 0xcc));
	L("stickerPanDeleteFg", "#ffffff");
	L("stickerPreviewBg",   rgba(QColor(0,0,0), 0xb0));
	// ── History text ─────────────────────────────────────────────────
	L("historyTextInFg",              rgb(p.text));
	L("historyTextInFgSelected",      "#ffffff");
	L("historyTextOutFg",             rgb(li(p.text, 5)));
	L("historyTextOutFgSelected",     "#ffffff");
	L("historyLinkInFg",              rgb(li(p.accent, 15)));
	L("historyLinkInFgSelected",      rgb(li(p.accent, 30)));
	L("historyLinkOutFg",             rgb(li(p.accent, 20)));
	L("historyLinkOutFgSelected",     rgb(li(p.accent, 30)));
	L("historyFileNameInFg",          rgb(p.text));
	L("historyFileNameInFgSelected",  "#ffffff");
	L("historyFileNameOutFg",         rgb(li(p.text, 5)));
	L("historyFileNameOutFgSelected", "#ffffff");
	L("historyOutIconFg",             rgb(li(p.accent, 20)));
	L("historyOutIconFgSelected",     "#ffffff");
	L("historyIconFgInverted",        rgba(QColor(0xff,0xff,0xff), 0xe5));
	L("historySendingOutIconFg",      rgb(da(p.accent, 15)));
	L("historySendingInIconFg",       rgb(p.textSub));
	L("historySendingInvertedIconFg", rgba(QColor(0xff,0xff,0xff), 0xc8));
	L("historyCallArrowInFg",              rgb(p.accent));
	L("historyCallArrowInFgSelected",      "#ffffff");
	L("historyCallArrowMissedInFg",        "#ed5050");
	L("historyCallArrowMissedInFgSelected","#ffffff");
	L("historyCallArrowOutFg",             "#ffffff");
	L("historyCallArrowOutFgSelected",     "#ffffff");
	L("historyUnreadBarBg",     rgb(da(p.surface, 5)));
	L("historyUnreadBarBorder", rgba(QColor(0,0,0), 0x00));
	L("historyUnreadBarFg",     "#ffffff");
	L("historyForwardChooseBg", rgba(QColor(0,0,0), 0x4c));
	L("historyForwardChooseFg", "#ffffff");
	// ── Peer name colors (fixed Telegram spec) ───────────────────────
	L("historyPeer1NameFg",          "#fb6169");
	L("historyPeer1NameFgSelected",  "#ffffff");
	L("historyPeer1UserpicBg",       "#ff845e");
	L("historyPeer2NameFg",          "#85de85");
	L("historyPeer2NameFgSelected",  "#ffffff");
	L("historyPeer2UserpicBg",       "#9ad164");
	L("historyPeer3NameFg",          "#f3bc5c");
	L("historyPeer3NameFgSelected",  "#ffffff");
	L("historyPeer3UserpicBg",       "#e5ca77");
	L("historyPeer4NameFg",          "#65bdf3");
	L("historyPeer4NameFgSelected",  "#ffffff");
	L("historyPeer4UserpicBg",       "#5caffa");
	L("historyPeer5NameFg",          "#b48bf2");
	L("historyPeer5NameFgSelected",  "#ffffff");
	L("historyPeer5UserpicBg",       "#b694f9");
	L("historyPeer6NameFg",          "#ff5694");
	L("historyPeer6NameFgSelected",  "#ffffff");
	L("historyPeer6UserpicBg",       "#ff8aac");
	L("historyPeer7NameFg",          "#62d4e3");
	L("historyPeer7NameFgSelected",  "#ffffff");
	L("historyPeer7UserpicBg",       "#5bcbe3");
	L("historyPeer8NameFg",          "#faa357");
	L("historyPeer8NameFgSelected",  "#ffffff");
	L("historyPeer8UserpicBg",       "#febb5b");
	L("historyPeerUserpicFg",        "#ffffff");
	L("historyPeer1UserpicBg2",      "#d45246");
	L("historyPeer2UserpicBg2",      "#46ba43");
	L("historyPeer3UserpicBg2",      "#e5ca77");
	L("historyPeer4UserpicBg2",      "#408acf");
	L("historyPeer5UserpicBg2",      "#6c61df");
	L("historyPeer6UserpicBg2",      "#d95574");
	L("historyPeer7UserpicBg2",      "#359ad4");
	L("historyPeer8UserpicBg2",      "#f68136");
	L("historyPeerSavedMessagesBg2", "#408acf");
	// ── History scroll bars ──────────────────────────────────────────
	L("historyScrollBarBg",     "#7f84897a");
	L("historyScrollBarBgOver", "#64686cbc");
	L("historyScrollBg",        "#565a5e4c");
	L("historyScrollBgOver",    "#5a5d616b");
	// ── Message bubbles ──────────────────────────────────────────────
	L("msgInBg",             rgb(p.msgIn));
	L("msgInBgSelected",     rgb(da(p.accent, 20)));
	L("msgOutBg",            rgb(p.msgOut));
	L("msgOutBgSelected",    rgb(da(p.accent, 20)));
	L("msgSelectOverlay",    rgba(p.accent, 0x4c));
	L("msgStickerOverlay",   rgba(p.accent, 0x7f));
	L("msgInServiceFg",          rgb(li(p.accent, 20)));
	L("msgInServiceFgSelected",  "#ffffff");
	L("msgOutServiceFg",         rgb(li(p.accent, 25)));
	L("msgOutServiceFgSelected", "#ffffff");
	L("msgInShadow",            rgba(QColor(0x74,0x8e,0xa2), 0x00));
	L("msgInShadowSelected",    rgba(QColor(0x53,0x8e,0xbb), 0x00));
	L("msgOutShadow",           "#00000000");
	L("msgOutShadowSelected",   rgba(p.accent, 0x00));
	L("msgInDateFg",            rgb(p.textSub));
	L("msgInDateFgSelected",    "#ffffff");
	L("msgOutDateFg",           rgb(li(p.textSub, 20)));
	L("msgOutDateFgSelected",   "#ffffff");
	L("msgServiceFg",           "#ffffff");
	{
		const QColor svcBase = da(p.surface, 5);
		L("msgServiceBg",         rgba(svcBase, 0xd5));
		L("msgServiceBgSelected", rgb(da(p.accent, 20)));
	}
	L("msgInReplyBarColor",    rgb(p.accent));
	L("msgInReplyBarSelColor", "#ffffff");
	L("msgOutReplyBarColor",   rgb(li(p.accent, 10)));
	L("msgOutReplyBarSelColor","#ffffff");
	L("msgImgReplyBarColor",   "#ffffff");
	L("msgInMonoFg",           rgb(da(p.accent, 5)));
	L("msgOutMonoFg",          rgb(li(p.accent, 20)));
	L("msgInMonoFgSelected",   rgb(li(p.accent, 30)));
	L("msgOutMonoFgSelected",  rgb(li(p.accent, 30)));
	L("msgDateImgFg",          rgba(QColor(0xff,0xff,0xff), 0xf2));
	L("msgDateImgBg",          rgba(QColor(0,0,0), 0x54));
	L("msgDateImgBgOver",      rgba(QColor(0,0,0), 0x74));
	L("msgDateImgBgSelected",  rgba(da(p.accent, 20), 0x87));
	L("msgFileThumbLinkInFg",           rgb(p.accent));
	L("msgFileThumbLinkInFgSelected",   "#ffffff");
	L("msgFileThumbLinkOutFg",          rgb(da(p.accent, 20)));
	L("msgFileThumbLinkOutFgSelected",  "#ffffff");
	L("msgFileInBg",            rgb(p.accent));
	L("msgFileInBgOver",        rgb(li(p.accent, 5)));
	L("msgFileInBgSelected",    rgb(li(p.accent, 15)));
	L("msgFileOutBg",           rgb(da(p.accent, 8)));
	L("msgFileOutBgOver",       rgb(da(p.accent, 3)));
	L("msgFileOutBgSelected",   rgb(p.accent));
	L("msgFile1Bg",             rgb(da(p.accent, 15)));
	L("msgFile1BgDark",         rgb(da(p.accent, 30)));
	L("msgFile1BgOver",         rgb(da(p.accent, 40)));
	L("msgFile1BgSelected",     "#ffffff");
	L("msgFile2Bg",             "#3ea34a");
	L("msgFile2BgDark",         "#298835");
	L("msgFile2BgOver",         "#1b7725");
	L("msgFile2BgSelected",     "#1b7725");
	L("msgFile3Bg",             "#d6454c");
	L("msgFile3BgDark",         "#bf333a");
	L("msgFile3BgOver",         "#b2282f");
	L("msgFile3BgSelected",     "#b2282f");
	L("msgFile4Bg",             "#d99546");
	L("msgFile4BgDark",         "#c17d39");
	L("msgFile4BgOver",         "#ac6b29");
	L("msgFile4BgSelected",     "#ac6b29");
	// ── File download icons ──────────────────────────────────────────
	L("historyFileInIconFg",             "#ffffff");
	L("historyFileInIconFgSelected",     "#ffffff");
	L("historyFileInRadialFg",           "#ffffff");
	L("historyFileInRadialFgSelected",   "#ffffff");
	L("historyFileOutIconFg",            "#ffffff");
	L("historyFileOutIconFgSelected",    "#ffffff");
	L("historyFileOutRadialFg",          "#ffffff");
	L("historyFileOutRadialFgSelected",  "#ffffff");
	L("historyFileThumbIconFg",          "#efefef");
	L("historyFileThumbIconFgSelected",  "#ffffff");
	L("historyFileThumbRadialFg",        "#efefef");
	L("historyFileThumbRadialFgSelected","#ffffff");
	L("historyVideoMessageProgressFg",   "#efefef");
	// ── Voice waveform ───────────────────────────────────────────────
	L("msgWaveformInActive",             rgb(p.accent));
	L("msgWaveformInActiveSelected",     "#ffffff");
	L("msgWaveformInInactive",           rgb(da(p.msgIn, 5)));
	L("msgWaveformInInactiveSelected",   rgb(da(p.accent, 20)));
	L("msgWaveformOutActive",            rgb(li(p.accent, 15)));
	L("msgWaveformOutActiveSelected",    "#ffffff");
	L("msgWaveformOutInactive",          rgb(da(p.accent, 30)));
	L("msgWaveformOutInactiveSelected",  rgb(da(p.accent, 15)));
	// ── Bot keyboard ─────────────────────────────────────────────────
	L("msgBotKbOverBgAdd",  "#80b1db0f");
	L("msgBotKbIconFg",     "#ffffff");
	L("msgBotKbRippleBg",   "#92c0e50b");
	// ── Media status text ────────────────────────────────────────────
	L("mediaInFg",          rgb(p.textSub));
	L("mediaInFgSelected",  rgb(p.textSub));
	L("mediaOutFg",         rgb(li(p.textSub, 15)));
	L("mediaOutFgSelected", rgb(li(p.textSub, 15)));
	// ── Video / youtube play icons ───────────────────────────────────
	L("youtubePlayIconBg",  "#e83131c8");
	L("youtubePlayIconFg",  "#ffffff");
	L("videoPlayIconBg",    rgba(QColor(0,0,0), 0x7f));
	L("videoPlayIconFg",    "#ffffff");
	// ── Toast ────────────────────────────────────────────────────────
	L("toastBg", rgba(QColor(0,0,0), 0xb2));
	L("toastFg", "#ffffff");
	// ── Report spam ──────────────────────────────────────────────────
	L("reportSpamBg", rgb(da(p.surface, 8)));
	L("reportSpamFg", rgb(p.text));
	// ── History scroll-to-down button ────────────────────────────────
	L("historyToDownBg",       rgb(p.card));
	L("historyToDownBgOver",   rgb(li(p.card, 8)));
	L("historyToDownBgRipple", rgb(li(p.card, 12)));
	L("historyToDownFg",       rgb(p.textSub));
	L("historyToDownFgOver",   "#dcdcdc");
	L("historyToDownShadow",   rgba(QColor(0,0,0), 0x40));
	// ── Compose area ─────────────────────────────────────────────────
	L("historyComposeAreaBg",         rgb(p.card));
	L("historyComposeAreaFg",         rgb(p.text));
	L("historyComposeAreaFgService",  rgb(p.textSub));
	L("historyComposeIconFg",         rgb(p.textSub));
	L("historyComposeIconFgOver",     "#dcdcdc");
	L("historySendIconFg",            rgb(p.accent));
	L("historySendIconFgOver",        rgb(p.accent));
	L("historyPinnedBg",              rgb(da(p.card, 5)));
	L("historyReplyBg",               rgb(p.card));
	L("historyReplyIconFg",           rgb(p.accent));
	L("historyReplyCancelFg",         rgb(p.textSub));
	L("historyReplyCancelFgOver",     "#dcdcdc");
	L("historyComposeButtonBg",       rgb(p.card));
	L("historyComposeButtonBgOver",   rgb(li(p.card, 5)));
	L("historyComposeButtonBgRipple", rgb(li(p.card, 10)));
	// ── Overview / shared media ──────────────────────────────────────
	L("overviewCheckBg",           rgba(QColor(0,0,0), 0x40));
	L("overviewCheckFg",           "#ffffff");
	L("overviewCheckFgActive",     "#ffffff");
	L("overviewPhotoSelectOverlay",rgba(p.accent, 0x33));
	// ── Profile ──────────────────────────────────────────────────────
	L("profileStatusFgOver",       rgb(p.textSub));
	L("profileVerifiedCheckBg",    rgb(p.accent));
	L("profileVerifiedCheckFg",    "#ffffff");
	L("profileAdminStartFg",       rgb(li(p.accent, 10)));
	// ── Notifications settings box ───────────────────────────────────
	L("notificationsBoxMonitorFg",   rgb(p.text));
	L("notificationsBoxScreenBg",    rgb(da(p.accent, 40)));
	L("notificationSampleUserpicFg", rgb(p.accent));
	L("notificationSampleCloseFg",   "#d7d7d7");
	L("notificationSampleTextFg",    "#d7d7d7");
	L("notificationSampleNameFg",    "#939393");
	L("changePhoneSimcardFrom",      "#d7d7d7");
	L("changePhoneSimcardTo",        "#939393");
	// ── Main menu ────────────────────────────────────────────────────
	L("mainMenuBg",       rgb(p.bg));
	L("mainMenuCoverBg",  rgb(da(p.accent, 25)));
	L("mainMenuCoverFg",  "#ffffff");
	L("mainMenuCloudFg",  "#ffffff");
	L("mainMenuCloudBg",  rgb(da(p.accent, 35)));
	// ── Media player ─────────────────────────────────────────────────
	L("mediaPlayerBg",         rgb(p.bg));
	L("mediaPlayerActiveFg",   rgb(p.accent));
	L("mediaPlayerInactiveFg", rgb(li(p.surface, 15)));
	L("mediaPlayerDisabledFg", rgb(li(p.accent, 30)));
	// ── Media viewer (fullscreen) ────────────────────────────────────
	L("mediaviewFileBg",             rgb(p.bg));
	L("mediaviewFileNameFg",         rgb(p.text));
	L("mediaviewFileSizeFg",         rgb(p.textSub));
	L("mediaviewFileRedCornerFg",    "#d55959");
	L("mediaviewFileYellowCornerFg", "#e8a659");
	L("mediaviewFileGreenCornerFg",  "#49a957");
	L("mediaviewFileBlueCornerFg",   "#599dcf");
	L("mediaviewFileExtFg",          "#ffffff");
	L("mediaviewMenuBg",             "#383838");
	L("mediaviewMenuBgOver",         "#505050");
	L("mediaviewMenuBgRipple",       "#676767");
	L("mediaviewMenuFg",             "#ffffff");
	L("mediaviewBg",                 rgba(QColor(0x22,0x22,0x22), 0xeb));
	L("mediaviewVideoBg",            "#000000");
	L("mediaviewControlBg",          rgba(QColor(0,0,0), 0x3c));
	L("mediaviewControlFg",          "#ffffff");
	L("mediaviewCaptionBg",          rgba(QColor(0x11,0x11,0x11), 0x80));
	L("mediaviewCaptionFg",          "#ffffff");
	L("mediaviewTextLinkFg",         rgb(accentLi));
	L("mediaviewSaveMsgBg",          rgba(QColor(0,0,0), 0xb2));
	L("mediaviewSaveMsgFg",          "#ffffff");
	L("mediaviewPlaybackActive",     "#c7c7c7");
	L("mediaviewPlaybackInactive",   "#252525");
	L("mediaviewPlaybackActiveOver", "#ffffff");
	L("mediaviewPlaybackInactiveOver","#474747");
	L("mediaviewPlaybackProgressFg", rgba(QColor(0xff,0xff,0xff), 0xc7));
	L("mediaviewPlaybackIconFg",     "#c7c7c7");
	L("mediaviewPlaybackIconFgOver", "#ffffff");
	L("mediaviewTransparentBg",      "#ffffff");
	L("mediaviewTransparentFg",      "#cccccc");
	// ── Notification popup window ────────────────────────────────────
	L("notificationBg", rgb(p.bg));
	// ── Call popup ───────────────────────────────────────────────────
	{
		const QColor callBg = da(p.bg, 5);
		L("callBg", rgba(callBg, 0xf5));
	}
	L("callNameFg",          "#ffffff");
	L("callFingerprintBg",   rgba(QColor(0,0,0), 0x66));
	L("callStatusFg",        "#aaabac");
	L("callIconFg",          "#ffffff");
	L("callAnswerBg",        rgb(p.accent));
	L("callAnswerRipple",    rgb(da(p.accent, 5)));
	L("callAnswerBgOuter",   rgba(p.accent, 0x26));
	L("callHangupBg",        "#cc4646");
	L("callHangupRipple",    "#ca4141");
	L("callCancelBg",        "#ffffff");
	L("callCancelFg",        rgb(p.textSub));
	L("callCancelRipple",    rgb(da(p.textSub, 10)));
	L("callMuteRipple",      rgba(QColor(0xff,0xff,0xff), 0x12));
	L("callBarBg",           rgb(da(p.accent, 30)));
	L("callBarMuteRipple",   rgb(da(p.accent, 15)));
	L("callBarBgMuted",      rgb(da(p.textSub, 30)));
	L("callBarUnmuteRipple", rgb(da(p.textSub, 15)));
	L("callBarFg",           "#ffffff");
	// ── Important tooltip ────────────────────────────────────────────
	L("importantTooltipBg",     rgba(QColor(0,0,0), 0xb2));
	L("importantTooltipFg",     "#ffffff");
	L("importantTooltipFgLink", rgb(accentLi));
	// ── Bot keyboard (inline) ────────────────────────────────────────
	L("botKbBg",     rgb(li(p.surface, 10)));
	L("botKbDownBg", rgb(li(p.surface, 15)));
	// ── Emoji category icons ─────────────────────────────────────────
	L("emojiIconFg",       rgb(p.textSub));
	L("emojiIconFgActive", rgb(p.accent));
	// ── Overview / shared media checkbox border ──────────────────────
	L("overviewCheckBorder", rgb(li(p.surface, 20)));
	// ── Side bar (folder filters) ────────────────────────────────────
	L("sideBarBg",           rgb(da(p.surface, 8)));
	L("sideBarBgActive",     rgb(li(p.surface, 10)));
	L("sideBarBgRipple",     rgb(li(p.surface, 5)));
	L("sideBarTextFg",       rgb(p.textSub));
	L("sideBarTextFgActive", rgb(li(p.accent, 15)));
	L("sideBarIconFg",       rgb(p.textSub));
	L("sideBarIconFgActive", rgb(li(p.accent, 5)));
	L("sideBarBadgeBg",      rgb(li(p.accent, 5)));
	L("sideBarBadgeBgMuted", rgb(p.textSub));
	// ── Statistics charts ────────────────────────────────────────────
	L("statisticsChartInactive", rgba(da(p.surface, 10), 0xc8));
	L("statisticsChartActive",   rgba(da(p.surface, 5),  0xd8));

	return text.toUtf8();
}

void ApplyTheme(const BgPalette &p) {
	if (Window::Theme::ApplyEditedPalette(DerivePalette(p))) {
		Core::App().settings().setBgThemeName(p.name);
		Core::App().saveSettingsDelayed();
	}
}

void ApplyBundledTheme(const QString &name) {
	for (const auto &t : BundledThemes()) {
		if (t.name == name) {
			ApplyTheme(t);
			return;
		}
	}
}

void RestoreTheme() {
	const auto name = Core::App().settings().bgThemeName();
	if (!name.isEmpty()) {
		ApplyBundledTheme(name);
	}
}

} // namespace BetterGram
