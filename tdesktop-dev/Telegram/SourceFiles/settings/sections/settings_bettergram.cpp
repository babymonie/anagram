/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "settings/sections/settings_bettergram.h"

#include "settings/settings_common_session.h"

#include "bettergram/bg_theme.h"
#include "bettergram/hub.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "lang/lang_keys.h"
#include "main/main_session.h"
#include "settings/settings_builder.h"
#include "settings/sections/settings_main.h"
#include "ui/layers/generic_box.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"
#include "styles/style_boxes.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"

#include <QtCore/QDateTime>

namespace Settings {
namespace {

using namespace Builder;

void ShowEditBox(
		not_null<Window::SessionController*> controller,
		Fn<rpl::producer<QString>()> makeTitle,
		Fn<rpl::producer<QString>()> makePlaceholder,
		Fn<rpl::producer<QString>()> makeAbout,
		Fn<QString()> getter,
		Fn<void(QString)> setter) {
	controller->show(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(makeTitle());
		const auto field = box->addRow(object_ptr<Ui::InputField>(
			box,
			st::defaultInputField,
			makePlaceholder(),
			getter()));
		Ui::AddSkip(box->verticalLayout());
		Ui::AddDividerText(box->verticalLayout(), makeAbout());
		const auto save = [=] {
			setter(field->getLastText().trimmed());
			Core::App().saveSettingsDelayed();
			box->closeBox();
		};
		field->submits() | rpl::on_next(save, field->lifetime());
		box->addButton(tr::lng_settings_save(), save);
		box->addButton(tr::lng_cancel(), [=] { box->closeBox(); });
	}));
}

[[nodiscard]] QString MaskKey(const QString &key) {
	return key.isEmpty() ? u"Not set"_q : u"Set"_q;
}

[[nodiscard]] QString ShowUrl(const QString &url) {
	return url.isEmpty() ? u"Not set"_q : url;
}

// Rebinds a snapshot-read value to BetterGram settings changes, so rows
// update live instead of only after Settings is closed and reopened.
template <typename T>
[[nodiscard]] rpl::producer<T> LiveValue(Fn<T()> getter) {
	return rpl::single(
		getter()
	) | rpl::then(
		Core::App().settings().bettergramChanges() | rpl::map(getter)
	);
}

void BuildHubSection(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addDivider();
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"bettergram/hub"_q,
		.title = tr::lng_bettergram_hub(),
		.keywords = { u"hub"_q, u"community"_q, u"server"_q, u"animated"_q },
	});

	// Hub URL
	builder.addButton({
		.id = u"bettergram/hub/url"_q,
		.title = tr::lng_bettergram_hub_url(),
		.icon = { &st::menuIconAdd },
		.label = LiveValue<QString>([] {
			return BetterGram::Hub::instance().isConfigured()
				? tr::lng_bettergram_hub_status_connected(tr::now)
				: tr::lng_bettergram_hub_status_none(tr::now);
		}),
		.onClick = [=] {
			ShowEditBox(
				controller,
				[] { return tr::lng_bettergram_hub_url(); },
				[] { return tr::lng_bettergram_hub_url_placeholder(); },
				[] { return tr::lng_bettergram_hub_url_about(); },
				[] { return BetterGram::Hub::instance().hubUrl(); },
				[](QString v) { BetterGram::Hub::instance().setHubUrl(v); });
		},
		.keywords = { u"hub"_q, u"server"_q, u"url"_q },
	});

	// Animated avatar URL
	builder.addButton({
		.id = u"bettergram/hub/avatar"_q,
		.title = tr::lng_bettergram_hub_avatar_url(),
		.icon = { &st::menuIconProfile },
		.label = LiveValue<QString>([] {
			return ShowUrl(Core::App().settings().bgAvatarUrl());
		}),
		.onClick = [=] {
			ShowEditBox(
				controller,
				[] { return tr::lng_bettergram_hub_avatar_url(); },
				[] { return tr::lng_bettergram_hub_avatar_placeholder(); },
				[] { return tr::lng_bettergram_hub_url_about(); },
				[] { return Core::App().settings().bgAvatarUrl(); },
				[](QString v) {
					Core::App().settings().setBgAvatarUrl(v);
					Core::App().saveSettingsDelayed();
				});
		},
		.keywords = { u"avatar"_q, u"gif"_q, u"animated"_q },
	});

	// Name colour
	builder.addButton({
		.id = u"bettergram/hub/color"_q,
		.title = tr::lng_bettergram_hub_color(),
		.icon = { &st::menuIconChangeColors },
		.label = LiveValue<QString>([] {
			return ShowUrl(Core::App().settings().bgNameColor());
		}),
		.onClick = [=] {
			ShowEditBox(
				controller,
				[] { return tr::lng_bettergram_hub_color(); },
				[] { return tr::lng_bettergram_hub_color_placeholder(); },
				[] { return tr::lng_bettergram_hub_url_about(); },
				[] { return Core::App().settings().bgNameColor(); },
				[](QString v) {
					Core::App().settings().setBgNameColor(v);
					Core::App().saveSettingsDelayed();
				});
		},
		.keywords = { u"color"_q, u"colour"_q, u"name"_q },
	});

	// Save to Hub button
	builder.addButton({
		.id = u"bettergram/hub/save"_q,
		.title = tr::lng_bettergram_hub_save(),
		.icon = { &st::menuIconSavedMessages },
		.label = rpl::single(QString()),
		.onClick = [=] {
			if (!BetterGram::Hub::instance().isConfigured()) {
				controller->showToast(
					tr::lng_bettergram_hub_not_configured(tr::now));
				return;
			}
			const auto session = &controller->session();
			BetterGram::HubProfile profile;
			profile.tgId = session->userId().bare;
			profile.avatarUrl = Core::App().settings().bgAvatarUrl();
			profile.color = Core::App().settings().bgNameColor();
			BetterGram::Hub::instance().setMyProfile(profile, [=](bool ok) {
				controller->showToast(ok
					? tr::lng_bettergram_hub_saved(tr::now)
					: tr::lng_bettergram_hub_save_failed(tr::now));
			});
		},
		.keywords = { u"hub"_q, u"save"_q, u"profile"_q, u"push"_q },
	});

	// View announcements — fetched from the hub, pinned first.
	builder.addButton({
		.id = u"bettergram/hub/announcements"_q,
		.title = tr::lng_bettergram_hub_announcements(),
		.icon = { &st::menuIconChannel },
		.label = rpl::single(QString()),
		.onClick = [=] {
			if (!BetterGram::Hub::instance().isConfigured()) {
				controller->showToast(
					tr::lng_bettergram_hub_not_configured(tr::now));
				return;
			}
			BetterGram::Hub::instance().fetchAnnouncements([=](
					QVector<BetterGram::HubAnnouncement> list) {
				ranges::sort(list, [](
						const BetterGram::HubAnnouncement &a,
						const BetterGram::HubAnnouncement &b) {
					return (a.pinned != b.pinned)
						? (a.pinned > b.pinned)
						: (a.createdAt > b.createdAt);
				});
				controller->show(Box([=](not_null<Ui::GenericBox*> box) {
					box->setTitle(tr::lng_bettergram_hub_announcements());
					if (list.isEmpty()) {
						box->addRow(object_ptr<Ui::FlatLabel>(
							box,
							tr::lng_bettergram_hub_announcements_empty(),
							st::aboutLabel));
					}
					for (const auto &a : list) {
						const auto title = a.pinned
							? (u"\U0001F4CC "_q + a.title)
							: a.title;
						box->addRow(object_ptr<Ui::FlatLabel>(
							box,
							rpl::single(title),
							st::boxTitle));
						box->addRow(object_ptr<Ui::FlatLabel>(
							box,
							rpl::single(a.body),
							st::boxDividerLabel));
						Ui::AddSkip(box->verticalLayout());
					}
					box->addButton(tr::lng_close(), [=] { box->closeBox(); });
				}));
			});
		},
		.keywords = { u"announcement"_q, u"news"_q, u"broadcast"_q, u"pinned"_q },
	});

	builder.addSkip();
}

void BuildTranslationSection(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"bettergram/translation"_q,
		.title = tr::lng_bettergram_translation(),
		.keywords = { u"translate"_q, u"server"_q, u"url"_q },
	});

	builder.addButton({
		.id = u"bettergram/translation/server"_q,
		.title = tr::lng_translate_settings_server(),
		.icon = { &st::menuIconTranslate },
		.label = LiveValue<QString>([] {
			return ShowUrl(Core::App().settings().translateUrlTemplate());
		}),
		.onClick = [=] {
			ShowEditBox(
				controller,
				[] { return tr::lng_translate_settings_server(); },
				[] { return tr::lng_translate_settings_server_placeholder(); },
				[] { return tr::lng_translate_settings_server_about(); },
				[] { return Core::App().settings().translateUrlTemplate(); },
				[](QString v) {
					Core::App().settings().setTranslateUrlTemplate(v);
					Core::App().saveSettingsDelayed();
				});
		},
		.keywords = { u"translate"_q, u"server"_q, u"url"_q, u"libretranslate"_q, u"lingva"_q },
	});

	builder.addSkip();
}

void BuildVoiceSection(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addDivider();
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"bettergram/voice"_q,
		.title = tr::lng_bettergram_voice(),
		.keywords = { u"whisper"_q, u"voice"_q, u"transcribe"_q, u"stt"_q },
	});

	builder.addButton({
		.id = u"bettergram/voice/url"_q,
		.title = tr::lng_whisper_api_url(),
		.icon = { &st::menuIconUnmute },
		.label = LiveValue<QString>([] {
			return ShowUrl(Core::App().settings().whisperApiUrl());
		}),
		.onClick = [=] {
			ShowEditBox(
				controller,
				[] { return tr::lng_whisper_api_url(); },
				[] { return tr::lng_whisper_api_url_placeholder(); },
				[] { return tr::lng_whisper_api_about(); },
				[] { return Core::App().settings().whisperApiUrl(); },
				[](QString v) { Core::App().settings().setWhisperApiUrl(v); });
		},
		.keywords = { u"whisper"_q, u"voice"_q, u"transcribe"_q, u"stt"_q, u"url"_q },
	});

	builder.addButton({
		.id = u"bettergram/voice/key"_q,
		.title = tr::lng_whisper_api_key(),
		.icon = { &st::menuIconLock },
		.label = LiveValue<QString>([] {
			return MaskKey(Core::App().settings().whisperApiKey());
		}),
		.onClick = [=] {
			ShowEditBox(
				controller,
				[] { return tr::lng_whisper_api_key(); },
				[] { return tr::lng_whisper_api_key_placeholder(); },
				[] { return tr::lng_whisper_api_about(); },
				[] { return Core::App().settings().whisperApiKey(); },
				[](QString v) { Core::App().settings().setWhisperApiKey(v); });
		},
		.keywords = { u"whisper"_q, u"key"_q, u"api"_q, u"voice"_q },
	});

	// Floating mini-player toggle
	if (const auto btn = builder.addButton({
			.id = u"bettergram/voice/mini_player"_q,
			.title = tr::lng_bettergram_mini_player(),
			.st = &st::settingsButtonNoIcon,
			.toggled = LiveValue<bool>([] {
				return Core::App().settings().bgMiniPlayerEnabled();
			}),
			.keywords = { u"mini"_q, u"player"_q, u"floating"_q, u"call"_q },
		})) {
		btn->toggledValue(
		) | rpl::filter([](bool checked) {
			return (checked != Core::App().settings().bgMiniPlayerEnabled());
		}) | rpl::on_next([](bool v) {
			Core::App().settings().setBgMiniPlayerEnabled(v);
			Core::App().saveSettingsDelayed();
		}, btn->lifetime());
	}
	builder.addDividerText(tr::lng_bettergram_mini_player_about());

	builder.addSkip();
}

void BuildAiComposeSection(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addDivider();
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"bettergram/ai_compose"_q,
		.title = tr::lng_bettergram_ai_compose(),
		.keywords = { u"ollama"_q, u"openai"_q, u"ai"_q, u"compose"_q, u"llm"_q },
	});

	builder.addButton({
		.id = u"bettergram/ai_compose/url"_q,
		.title = tr::lng_ai_compose_api_url(),
		.icon = { &st::menuIconManage },
		.label = LiveValue<QString>([] {
			return ShowUrl(Core::App().settings().aiComposeApiUrl());
		}),
		.onClick = [=] {
			ShowEditBox(
				controller,
				[] { return tr::lng_ai_compose_api_url(); },
				[] { return tr::lng_ai_compose_api_url_placeholder(); },
				[] { return tr::lng_ai_compose_api_about(); },
				[] { return Core::App().settings().aiComposeApiUrl(); },
				[](QString v) { Core::App().settings().setAiComposeApiUrl(v); });
		},
		.keywords = { u"ollama"_q, u"openai"_q, u"ai"_q, u"compose"_q, u"llm"_q, u"url"_q },
	});

	builder.addButton({
		.id = u"bettergram/ai_compose/key"_q,
		.title = tr::lng_ai_compose_api_key(),
		.icon = { &st::menuIconLock },
		.label = LiveValue<QString>([] {
			return MaskKey(Core::App().settings().aiComposeApiKey());
		}),
		.onClick = [=] {
			ShowEditBox(
				controller,
				[] { return tr::lng_ai_compose_api_key(); },
				[] { return tr::lng_ai_compose_api_key_placeholder(); },
				[] { return tr::lng_ai_compose_api_about(); },
				[] { return Core::App().settings().aiComposeApiKey(); },
				[](QString v) { Core::App().settings().setAiComposeApiKey(v); });
		},
		.keywords = { u"ai"_q, u"key"_q, u"compose"_q, u"api"_q },
	});

	builder.addButton({
		.id = u"bettergram/ai_compose/model"_q,
		.title = tr::lng_ai_compose_api_model(),
		.icon = { &st::menuIconChatBubble },
		.label = LiveValue<QString>([] {
			const auto model = Core::App().settings().aiComposeApiModel();
			return model.isEmpty() ? u"Default"_q : model;
		}),
		.onClick = [=] {
			ShowEditBox(
				controller,
				[] { return tr::lng_ai_compose_api_model(); },
				[] { return tr::lng_ai_compose_api_model_placeholder(); },
				[] { return tr::lng_ai_compose_api_about(); },
				[] { return Core::App().settings().aiComposeApiModel(); },
				[](QString v) { Core::App().settings().setAiComposeApiModel(v); });
		},
		.keywords = { u"ai"_q, u"model"_q, u"llama"_q, u"gpt"_q, u"compose"_q },
	});

	builder.addSkip();
}

void BuildStoriesSection(SectionBuilder &builder) {
	builder.addDivider();
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"bettergram/stories"_q,
		.title = tr::lng_bettergram_stories(),
		.keywords = { u"stories"_q, u"bar"_q, u"badge"_q, u"ring"_q },
	});

	const auto hideBar = builder.addButton({
		.id = u"bettergram/stories/hide_bar"_q,
		.title = tr::lng_bettergram_hide_stories_bar(),
		.st = &st::settingsButtonNoIcon,
		.toggled = LiveValue<bool>([] {
			return Core::App().settings().hideStoriesBar();
		}),
		.keywords = { u"stories"_q, u"bar"_q, u"hide"_q },
	});
	if (hideBar) {
		hideBar->toggledValue(
		) | rpl::filter([](bool checked) {
			return (checked != Core::App().settings().hideStoriesBar());
		}) | rpl::on_next([](bool checked) {
			Core::App().settings().setHideStoriesBar(checked);
			Core::App().saveSettingsDelayed();
		}, hideBar->lifetime());
	}

	const auto hideBadges = builder.addButton({
		.id = u"bettergram/stories/hide_badges"_q,
		.title = tr::lng_bettergram_hide_story_badges(),
		.st = &st::settingsButtonNoIcon,
		.toggled = LiveValue<bool>([] {
			return Core::App().settings().hideStoryBadges();
		}),
		.keywords = { u"stories"_q, u"badge"_q, u"ring"_q, u"hide"_q },
	});
	if (hideBadges) {
		hideBadges->toggledValue(
		) | rpl::filter([](bool checked) {
			return (checked != Core::App().settings().hideStoryBadges());
		}) | rpl::on_next([](bool checked) {
			Core::App().settings().setHideStoryBadges(checked);
			Core::App().saveSettingsDelayed();
		}, hideBadges->lifetime());
	}

	const auto hideNotifs = builder.addButton({
		.id = u"bettergram/stories/hide_notifications"_q,
		.title = tr::lng_bettergram_hide_story_notifications(),
		.st = &st::settingsButtonNoIcon,
		.toggled = LiveValue<bool>([] {
			return Core::App().settings().hideStoryNotifications();
		}),
		.keywords = { u"stories"_q, u"unread"_q, u"indicator"_q, u"hide"_q },
	});
	if (hideNotifs) {
		hideNotifs->toggledValue(
		) | rpl::filter([](bool checked) {
			return (checked != Core::App().settings().hideStoryNotifications());
		}) | rpl::on_next([](bool checked) {
			Core::App().settings().setHideStoryNotifications(checked);
			Core::App().saveSettingsDelayed();
		}, hideNotifs->lifetime());
	}

	builder.addSkip();
}

void BuildPremiumHideSection(SectionBuilder &builder) {
	builder.addDivider();
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"bettergram/premium"_q,
		.title = tr::lng_bettergram_premium(),
		.keywords = { u"premium"_q, u"badge"_q, u"upsell"_q, u"upgrade"_q },
	});

	const auto hideBadges = builder.addButton({
		.id = u"bettergram/premium/hide_badges"_q,
		.title = tr::lng_bettergram_hide_premium_badges(),
		.st = &st::settingsButtonNoIcon,
		.toggled = LiveValue<bool>([] {
			return Core::App().settings().hidePremiumBadges();
		}),
		.keywords = { u"premium"_q, u"badge"_q, u"star"_q, u"hide"_q },
	});
	if (hideBadges) {
		hideBadges->toggledValue(
		) | rpl::filter([](bool checked) {
			return (checked != Core::App().settings().hidePremiumBadges());
		}) | rpl::on_next([](bool checked) {
			Core::App().settings().setHidePremiumBadges(checked);
			Core::App().saveSettingsDelayed();
		}, hideBadges->lifetime());
	}

	const auto hideUpsells = builder.addButton({
		.id = u"bettergram/premium/hide_upsells"_q,
		.title = tr::lng_bettergram_hide_premium_upsells(),
		.st = &st::settingsButtonNoIcon,
		.toggled = LiveValue<bool>([] {
			return Core::App().settings().hidePremiumUpsells();
		}),
		.keywords = { u"premium"_q, u"popup"_q, u"upsell"_q, u"hide"_q },
	});
	if (hideUpsells) {
		hideUpsells->toggledValue(
		) | rpl::filter([](bool checked) {
			return (checked != Core::App().settings().hidePremiumUpsells());
		}) | rpl::on_next([](bool checked) {
			Core::App().settings().setHidePremiumUpsells(checked);
			Core::App().saveSettingsDelayed();
		}, hideUpsells->lifetime());
	}

	const auto hideMenuItems = builder.addButton({
		.id = u"bettergram/premium/hide_menu_items"_q,
		.title = tr::lng_bettergram_hide_premium_menu_items(),
		.st = &st::settingsButtonNoIcon,
		.toggled = LiveValue<bool>([] {
			return Core::App().settings().hidePremiumMenuItems();
		}),
		.keywords = { u"premium"_q, u"menu"_q, u"gift"_q, u"hide"_q },
	});
	if (hideMenuItems) {
		hideMenuItems->toggledValue(
		) | rpl::filter([](bool checked) {
			return (checked != Core::App().settings().hidePremiumMenuItems());
		}) | rpl::on_next([](bool checked) {
			Core::App().settings().setHidePremiumMenuItems(checked);
			Core::App().saveSettingsDelayed();
		}, hideMenuItems->lifetime());
	}

	const auto hideUpgrade = builder.addButton({
		.id = u"bettergram/premium/hide_upgrade_buttons"_q,
		.title = tr::lng_bettergram_hide_upgrade_buttons(),
		.st = &st::settingsButtonNoIcon,
		.toggled = LiveValue<bool>([] {
			return Core::App().settings().hideUpgradeButtons();
		}),
		.keywords = { u"premium"_q, u"upgrade"_q, u"get"_q, u"button"_q, u"hide"_q },
	});
	if (hideUpgrade) {
		hideUpgrade->toggledValue(
		) | rpl::filter([](bool checked) {
			return (checked != Core::App().settings().hideUpgradeButtons());
		}) | rpl::on_next([](bool checked) {
			Core::App().settings().setHideUpgradeButtons(checked);
			Core::App().saveSettingsDelayed();
		}, hideUpgrade->lifetime());
	}

	const auto hideAds = builder.addButton({
		.id = u"bettergram/premium/hide_sponsored"_q,
		.title = tr::lng_bettergram_hide_sponsored_messages(),
		.st = &st::settingsButtonNoIcon,
		.toggled = LiveValue<bool>([] {
			return Core::App().settings().hideSponsoredMessages();
		}),
		.keywords = { u"ads"_q, u"sponsored"_q, u"hide"_q, u"channel"_q },
	});
	if (hideAds) {
		hideAds->toggledValue(
		) | rpl::filter([](bool checked) {
			return (checked != Core::App().settings().hideSponsoredMessages());
		}) | rpl::on_next([](bool checked) {
			Core::App().settings().setHideSponsoredMessages(checked);
			Core::App().saveSettingsDelayed();
		}, hideAds->lifetime());
	}

	builder.addSkip();
}

void BuildHubRolesSection(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addDivider();
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"bettergram/hub_roles"_q,
		.title = tr::lng_bettergram_hub_roles(),
		.keywords = { u"hub"_q, u"role"_q, u"badge"_q, u"admin"_q, u"mod"_q },
	});
	builder.addDividerText(tr::lng_bettergram_hub_roles_about());

	// Admin token
	builder.addButton({
		.id = u"bettergram/hub_roles/token"_q,
		.title = tr::lng_bettergram_hub_admin_token(),
		.icon = { &st::menuIconLock },
		.label = LiveValue<QString>([] {
			return MaskKey(Core::App().settings().bgHubAdminToken());
		}),
		.onClick = [=] {
			ShowEditBox(
				controller,
				[] { return tr::lng_bettergram_hub_admin_token(); },
				[] { return tr::lng_bettergram_hub_admin_token_placeholder(); },
				[] { return tr::lng_bettergram_hub_roles_about(); },
				[] { return Core::App().settings().bgHubAdminToken(); },
				[](QString v) {
					Core::App().settings().setBgHubAdminToken(v);
					Core::App().saveSettingsDelayed();
				});
		},
		.keywords = { u"admin"_q, u"token"_q, u"secret"_q },
	});

	// Assign role button — opens a box with tg_id, badge, color fields
	builder.addButton({
		.id = u"bettergram/hub_roles/assign"_q,
		.title = tr::lng_bettergram_hub_role_assign(),
		.icon = { &st::menuIconAdd },
		.label = rpl::single(QString()),
		.onClick = [=] {
			if (!BetterGram::Hub::instance().isConfigured()) {
				controller->showToast(tr::lng_bettergram_hub_not_configured(tr::now));
				return;
			}
			const auto token = Core::App().settings().bgHubAdminToken();
			if (token.isEmpty()) {
				controller->showToast(tr::lng_bettergram_hub_admin_token(tr::now));
				return;
			}
			controller->show(Box([=](not_null<Ui::GenericBox*> box) {
				box->setTitle(tr::lng_bettergram_hub_role_assign());
				const auto idField = box->addRow(object_ptr<Ui::InputField>(
					box,
					st::defaultInputField,
					tr::lng_bettergram_hub_role_tg_id_placeholder(),
					QString()));
				const auto badgeField = box->addRow(object_ptr<Ui::InputField>(
					box,
					st::defaultInputField,
					tr::lng_bettergram_hub_role_badge_placeholder(),
					QString()));
				const auto colorField = box->addRow(object_ptr<Ui::InputField>(
					box,
					st::defaultInputField,
					tr::lng_bettergram_hub_role_color_placeholder(),
					u"#5B7FF1"_q));
				Ui::AddSkip(box->verticalLayout());
				const auto assign = [=] {
					bool ok = false;
					const auto tgId = idField->getLastText().trimmed().toLongLong(&ok);
					if (!ok || tgId <= 0) {
						controller->showToast(tr::lng_bettergram_hub_role_tg_id(tr::now));
						return;
					}
					const auto badge = badgeField->getLastText().trimmed();
					const auto color = colorField->getLastText().trimmed();
					BetterGram::Hub::instance().setAdminRole(
						tgId, badge, color, token,
						[=](bool success) {
							controller->showToast(success
								? tr::lng_bettergram_hub_role_assigned(tr::now)
								: tr::lng_bettergram_hub_role_failed(tr::now));
						});
					box->closeBox();
				};
				box->addButton(tr::lng_bettergram_hub_role_assign(), assign);
				box->addButton(tr::lng_cancel(), [=] { box->closeBox(); });
			}));
		},
		.keywords = { u"role"_q, u"badge"_q, u"assign"_q },
	});

	// Remove role button
	builder.addButton({
		.id = u"bettergram/hub_roles/remove"_q,
		.title = tr::lng_bettergram_hub_role_remove(),
		.icon = { &st::menuIconDelete },
		.label = rpl::single(QString()),
		.onClick = [=] {
			if (!BetterGram::Hub::instance().isConfigured()) {
				controller->showToast(tr::lng_bettergram_hub_not_configured(tr::now));
				return;
			}
			const auto token = Core::App().settings().bgHubAdminToken();
			if (token.isEmpty()) {
				controller->showToast(tr::lng_bettergram_hub_admin_token(tr::now));
				return;
			}
			controller->show(Box([=](not_null<Ui::GenericBox*> box) {
				box->setTitle(tr::lng_bettergram_hub_role_remove());
				const auto idField = box->addRow(object_ptr<Ui::InputField>(
					box,
					st::defaultInputField,
					tr::lng_bettergram_hub_role_tg_id_placeholder(),
					QString()));
				const auto remove = [=] {
					bool ok = false;
					const auto tgId = idField->getLastText().trimmed().toLongLong(&ok);
					if (!ok || tgId <= 0) {
						controller->showToast(tr::lng_bettergram_hub_role_tg_id(tr::now));
						return;
					}
					BetterGram::Hub::instance().removeAdminRole(
						tgId, token,
						[=](bool success) {
							controller->showToast(success
								? tr::lng_bettergram_hub_role_removed(tr::now)
								: tr::lng_bettergram_hub_role_failed(tr::now));
						});
					box->closeBox();
				};
				box->addButton(tr::lng_bettergram_hub_role_remove(), remove);
				box->addButton(tr::lng_cancel(), [=] { box->closeBox(); });
			}));
		},
		.keywords = { u"role"_q, u"badge"_q, u"remove"_q, u"clear"_q },
	});

	builder.addSkip();
}

void BuildPowerSection(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addDivider();
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"bettergram/power"_q,
		.title = tr::lng_bettergram_power(),
		.keywords = {
			u"ghost"_q, u"delete"_q, u"timestamp"_q, u"typing"_q,
			u"counter"_q, u"tracker"_q, u"note"_q, u"sound"_q,
		},
	});
	builder.addDividerText(tr::lng_bettergram_power_about());

	// Ghost Messages toggle
	if (const auto btn = builder.addButton({
			.id = u"bettergram/power/ghost"_q,
			.title = tr::lng_bettergram_ghost_messages(),
			.st = &st::settingsButtonNoIcon,
			.toggled = LiveValue<bool>([] {
				return Core::App().settings().bgGhostMessages();
			}),
			.keywords = { u"ghost"_q, u"delete"_q, u"recover"_q },
		})) {
		btn->toggledValue(
		) | rpl::filter([](bool v) {
			return (v != Core::App().settings().bgGhostMessages());
		}) | rpl::on_next([](bool v) {
			Core::App().settings().setBgGhostMessages(v);
			Core::App().saveSettingsDelayed();
		}, btn->lifetime());
	}

	// Ghost Messages vault — browse/search kept-deleted messages, and
	// permanently remove entries (content otherwise survives independent
	// of Telegram's own history/cache lifetime).
	builder.addButton({
		.id = u"bettergram/power/ghost_vault"_q,
		.title = tr::lng_bettergram_ghost_vault(),
		.st = &st::settingsButtonNoIcon,
		.label = rpl::single(QString()),
		.onClick = [=] {
			const auto session = &controller->session();
			controller->show(Box([=](not_null<Ui::GenericBox*> box) {
				box->setTitle(tr::lng_bettergram_ghost_vault());
				const auto &vault = session->data().bgGhostVault();
				if (vault.empty()) {
					box->addRow(object_ptr<Ui::FlatLabel>(
						box,
						tr::lng_bettergram_ghost_vault_empty(),
						st::aboutLabel));
				}
				auto entries = std::vector<FullMsgId>();
				for (const auto &[id, content] : vault) {
					entries.push_back(id);
				}
				for (const auto &id : entries) {
					const auto i = vault.find(id);
					if (i == vault.end()) {
						continue;
					}
					const auto &content = i->second;
					const auto chatName = session->data().peer(
						id.peer)->name();
					const auto when = QDateTime::fromSecsSinceEpoch(
						content.date
					).toString(Qt::TextDate);
					box->addRow(object_ptr<Ui::FlatLabel>(
						box,
						rpl::single(
							chatName + u" – "_q + content.senderName
								+ u" – "_q + when),
						st::boxTitle));
					box->addRow(object_ptr<Ui::FlatLabel>(
						box,
						rpl::single(content.text),
						st::boxDividerLabel));
					box->addRow(object_ptr<Ui::LinkButton>(
						box,
						tr::lng_bettergram_ghost_vault_delete(tr::now)
					))->setClickedCallback([=] {
						session->data().removeBgGhostVaultEntry(id);
						box->closeBox();
					});
					Ui::AddSkip(box->verticalLayout());
				}
				box->addButton(tr::lng_close(), [=] { box->closeBox(); });
			}));
		},
		.keywords = { u"ghost"_q, u"vault"_q, u"deleted"_q, u"recover"_q },
	});

	// Always-show timestamps toggle
	if (const auto btn = builder.addButton({
			.id = u"bettergram/power/timestamps"_q,
			.title = tr::lng_bettergram_always_timestamps(),
			.st = &st::settingsButtonNoIcon,
			.toggled = LiveValue<bool>([] {
				return Core::App().settings().bgAlwaysShowTimestamps();
			}),
			.keywords = { u"timestamp"_q, u"time"_q, u"always"_q },
		})) {
		btn->toggledValue(
		) | rpl::filter([](bool v) {
			return (v != Core::App().settings().bgAlwaysShowTimestamps());
		}) | rpl::on_next([](bool v) {
			Core::App().settings().setBgAlwaysShowTimestamps(v);
			Core::App().saveSettingsDelayed();
		}, btn->lifetime());
	}

	// Character counter toggle
	if (const auto btn = builder.addButton({
			.id = u"bettergram/power/char_counter"_q,
			.title = tr::lng_bettergram_char_counter(),
			.st = &st::settingsButtonNoIcon,
			.toggled = LiveValue<bool>([] {
				return Core::App().settings().bgCharCounter();
			}),
			.keywords = { u"character"_q, u"counter"_q, u"count"_q, u"limit"_q },
		})) {
		btn->toggledValue(
		) | rpl::filter([](bool v) {
			return (v != Core::App().settings().bgCharCounter());
		}) | rpl::on_next([](bool v) {
			Core::App().settings().setBgCharCounter(v);
			Core::App().saveSettingsDelayed();
		}, btn->lifetime());
	}

	// Link de-tracker toggle
	if (const auto btn = builder.addButton({
			.id = u"bettergram/power/link_detracker"_q,
			.title = tr::lng_bettergram_link_detracker(),
			.st = &st::settingsButtonNoIcon,
			.toggled = LiveValue<bool>([] {
				return Core::App().settings().bgLinkDeTracker();
			}),
			.keywords = { u"link"_q, u"utm"_q, u"tracker"_q, u"privacy"_q },
		})) {
		btn->toggledValue(
		) | rpl::filter([](bool v) {
			return (v != Core::App().settings().bgLinkDeTracker());
		}) | rpl::on_next([](bool v) {
			Core::App().settings().setBgLinkDeTracker(v);
			Core::App().saveSettingsDelayed();
		}, btn->lifetime());
	}

	// Typing suppressor — manage suppressed peers
	builder.addButton({
		.id = u"bettergram/power/typing_suppress"_q,
		.title = tr::lng_bettergram_typing_suppress(),
		.icon = { &st::menuIconChatBubble },
		.label = LiveValue<QString>([] {
			return QString::number(
				Core::App().settings().bgTypingSuppressedPeers().size())
				+ u" peer(s)"_q;
		}),
		.onClick = [=] {
			controller->show(Box([=](not_null<Ui::GenericBox*> box) {
				box->setTitle(tr::lng_bettergram_typing_suppress());
				box->addRow(object_ptr<Ui::FlatLabel>(
					box,
					tr::lng_bettergram_typing_suppress_about(),
					st::aboutLabel));
				Ui::AddSkip(box->verticalLayout());

				// Show current list + remove section
				const auto peers =
					Core::App().settings().bgTypingSuppressedPeers();
				Fn<void()> removeAction;
				if (!peers.isEmpty()) {
					QStringList parts;
					for (const auto id : peers) {
						parts.push_back(QString::number(id));
					}
					box->addRow(object_ptr<Ui::FlatLabel>(
						box,
						rpl::single(
							tr::lng_bettergram_typing_suppress_current(tr::now)
							+ u": "_q + parts.join(u", "_q)),
						st::boxDividerLabel));
					Ui::AddSkip(box->verticalLayout());
					const auto removeField = box->addRow(
						object_ptr<Ui::InputField>(
							box,
							st::defaultInputField,
							tr::lng_bettergram_typing_suppress_remove_placeholder(),
							QString()));
					removeAction = [=] {
						bool ok = false;
						const auto id = removeField->getLastText()
							.trimmed().toULongLong(&ok);
						if (!ok || id == 0) return;
						Core::App().settings()
							.removeBgTypingSuppressedPeer(id);
						Core::App().saveSettingsDelayed();
						controller->showToast(
							tr::lng_bettergram_typing_suppress_removed(
								tr::now));
						box->closeBox();
					};
					removeField->submits() | rpl::on_next(
						removeAction, removeField->lifetime());
					Ui::AddDivider(box->verticalLayout());
					Ui::AddSkip(box->verticalLayout());
				}

				// Add section
				const auto addField = box->addRow(
					object_ptr<Ui::InputField>(
						box,
						st::defaultInputField,
						tr::lng_bettergram_typing_suppress_placeholder(),
						QString()));
				Ui::AddDividerText(
					box->verticalLayout(),
					tr::lng_bettergram_typing_suppress_hint());
				const auto add = [=] {
					bool ok = false;
					const auto id = addField->getLastText()
						.trimmed().toULongLong(&ok);
					if (!ok || id == 0) return;
					Core::App().settings().addBgTypingSuppressedPeer(id);
					Core::App().saveSettingsDelayed();
					controller->showToast(
						tr::lng_bettergram_typing_suppress_added(tr::now));
					box->closeBox();
				};
				addField->submits() | rpl::on_next(add, addField->lifetime());
				box->addButton(tr::lng_bettergram_typing_suppress_add(), add);
				if (removeAction) {
					box->addButton(
						tr::lng_bettergram_typing_suppress_remove(),
						removeAction);
				}
				box->addButton(tr::lng_cancel(), [=] { box->closeBox(); });
			}));
		},
		.keywords = { u"typing"_q, u"suppress"_q, u"silent"_q },
	});

	// Custom notification sound — manage per-peer sounds
	builder.addButton({
		.id = u"bettergram/power/custom_sound"_q,
		.title = tr::lng_bettergram_custom_sound(),
		.icon = { &st::menuIconSoundOn },
		.label = rpl::single(QString()),
		.onClick = [=] {
			controller->show(Box([=](not_null<Ui::GenericBox*> box) {
				box->setTitle(tr::lng_bettergram_custom_sound());
				box->addRow(object_ptr<Ui::FlatLabel>(
					box,
					tr::lng_bettergram_custom_sound_about(),
					st::aboutLabel));
				Ui::AddSkip(box->verticalLayout());
				const auto idField = box->addRow(object_ptr<Ui::InputField>(
					box,
					st::defaultInputField,
					tr::lng_bettergram_custom_sound_id_placeholder(),
					QString()));
				const auto pathField = box->addRow(object_ptr<Ui::InputField>(
					box,
					st::defaultInputField,
					tr::lng_bettergram_custom_sound_path_placeholder(),
					QString()));
				const auto save = [=] {
					bool ok = false;
					const auto id = idField->getLastText()
						.trimmed()
						.toULongLong(&ok);
					if (!ok || id == 0) {
						return;
					}
					Core::App().settings().setBgPeerSound(
						id,
						pathField->getLastText().trimmed());
					Core::App().saveSettingsDelayed();
					controller->showToast(
						tr::lng_bettergram_custom_sound_saved(tr::now));
					box->closeBox();
				};
				box->addButton(tr::lng_settings_save(), save);
				box->addButton(tr::lng_cancel(), [=] { box->closeBox(); });
			}));
		},
		.keywords = { u"sound"_q, u"notification"_q, u"ringtone"_q },
	});

	builder.addSkip();
}

void BuildThemeSection(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addDivider();
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"bettergram/theme"_q,
		.title = tr::lng_bettergram_theme(),
		.keywords = {
			u"theme"_q, u"color"_q, u"dark"_q, u"nord"_q,
			u"amoled"_q, u"gruvbox"_q, u"catppuccin"_q,
		},
	});
	builder.addDividerText(tr::lng_bettergram_theme_about());

	// Hub theme sync
	if (const auto btn = builder.addButton({
			.id = u"bettergram/theme/hub_sync"_q,
			.title = tr::lng_bettergram_hub_theme_sync(),
			.st = &st::settingsButtonNoIcon,
			.toggled = LiveValue<bool>([] {
				return Core::App().settings().bgHubThemeSync();
			}),
			.keywords = { u"hub"_q, u"theme"_q, u"sync"_q, u"server"_q },
		})) {
		btn->toggledValue(
		) | rpl::filter([](bool v) {
			return (v != Core::App().settings().bgHubThemeSync());
		}) | rpl::on_next([](bool v) {
			Core::App().settings().setBgHubThemeSync(v);
			Core::App().saveSettingsDelayed();
		}, btn->lifetime());
	}
	builder.addDividerText(tr::lng_bettergram_hub_theme_sync_about());

	for (const auto &theme : BetterGram::BundledThemes()) {
		const auto name = theme.name;
		builder.addButton({
			.id = u"bettergram/theme/%1"_q.arg(
				name.toLower().replace(QChar(' '), QChar('_'))),
			.title = rpl::single(name),
			.icon = { &st::menuIconChangeColors },
			.label = LiveValue<QString>([name] {
				return (Core::App().settings().bgThemeName() == name)
					? tr::lng_bettergram_theme_active(tr::now)
					: QString();
			}),
			.onClick = [=] {
				BetterGram::ApplyBundledTheme(name);
				controller->showToast(tr::lng_bettergram_theme_applied(tr::now));
			},
			.keywords = { u"theme"_q, u"color"_q, u"palette"_q },
		});
	}

	builder.addButton({
		.id = u"bettergram/theme/reset"_q,
		.title = tr::lng_bettergram_theme_reset(),
		.icon = { &st::menuIconDelete },
		.label = rpl::single(QString()),
		.onClick = [=] {
			Core::App().settings().setBgThemeName(QString());
			Core::App().saveSettingsDelayed();
			controller->showToast(tr::lng_bettergram_theme_reset_done(tr::now));
		},
		.keywords = { u"theme"_q, u"reset"_q, u"default"_q, u"clear"_q },
	});

	builder.addSkip();
}

void BuildUnlocksSection(SectionBuilder &builder) {
	builder.addDivider();
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"bettergram/unlocks"_q,
		.title = tr::lng_bettergram_unlocks(),
		.keywords = { u"premium"_q, u"unlock"_q, u"free"_q, u"translate"_q, u"wallpaper"_q, u"tags"_q },
	});
	builder.addDividerText(tr::lng_bettergram_unlocks_about());
	builder.addSkip();
}

class BetterGram final : public Section<BetterGram> {
public:
	BetterGram(
		QWidget *parent,
		not_null<Window::SessionController*> controller);

	[[nodiscard]] rpl::producer<QString> title() override;

private:
	void setupContent();

};

const auto kMeta = BuildHelper({
	.id = BetterGram::Id(),
	.parentId = MainId(),
	.title = &tr::lng_bettergram_section,
	.icon = &st::menuIconManage,
}, [](SectionBuilder &builder) {
	BuildHubSection(builder);
	BuildHubRolesSection(builder);
	BuildThemeSection(builder);
	BuildStoriesSection(builder);
	BuildPremiumHideSection(builder);
	BuildTranslationSection(builder);
	BuildVoiceSection(builder);
	BuildAiComposeSection(builder);
	BuildPowerSection(builder);
	BuildUnlocksSection(builder);
});

const SectionBuildMethod kBetterGramSection = kMeta.build;

BetterGram::BetterGram(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

rpl::producer<QString> BetterGram::title() {
	return tr::lng_bettergram_section();
}

void BetterGram::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);

	build(content, kBetterGramSection);

	Ui::ResizeFitChild(this, content);
}

} // namespace

Type BetterGramId() {
	return BetterGram::Id();
}

} // namespace Settings
