/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "lang/translate_provider.h"

#include "base/options.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "data/data_msg_id.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "history/history_item.h"
#include "lang/translate_mtproto_provider.h"
#include "lang/translate_url_provider.h"
#include "platform/platform_translate_provider.h"

namespace {

base::options::option<QString> OptionTranslateUrlTemplate({
	.id = "translate-url-template",
	.name = "Translate URL template",
	.description = "Template URL for custom translation provider."
		" Supports %q text, %f source language and %t target language."
		" If the URL has no %q, it is treated as a LibreTranslate-style"
		" server base URL and POSTed to as JSON (e.g. http://localhost:5000).",
});

} // namespace

namespace Ui {

std::unique_ptr<TranslateProvider> CreateTranslateProvider(
		not_null<Main::Session*> session) {
	const auto settingsUrl = Core::App().settings().translateUrlTemplate();
	if (!settingsUrl.isEmpty()) {
		return CreateUrlTranslateProvider(settingsUrl);
	}
	const auto optionUrl = OptionTranslateUrlTemplate.value();
	if (!optionUrl.isEmpty()) {
		return CreateUrlTranslateProvider(optionUrl);
	}
	if (Core::App().settings().usePlatformTranslation()
		&& Platform::IsTranslateProviderAvailable()) {
		return Platform::CreateTranslateProvider();
	}
	return CreateMTProtoTranslateProvider(session);
}

TranslateProviderRequest PrepareTranslateProviderRequest(
		not_null<TranslateProvider*> provider,
		not_null<PeerData*> peer,
		MsgId msgId,
		TextWithEntities text) {
	auto result = TranslateProviderRequest{
		.peerId = uint64(peer->id.value),
		.msgId = IsServerMsgId(msgId) ? msgId.bare : 0,
		.text = std::move(text),
	};
	if (provider->supportsMessageId()) {
		return result;
	}
	if (result.msgId) {
		if (const auto i = peer->owner().message(peer, MsgId(result.msgId))) {
			result.text = i->originalText();
		}
		result.msgId = 0;
	}
	return result;
}

} // namespace Ui
