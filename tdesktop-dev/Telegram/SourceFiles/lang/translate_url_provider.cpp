/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "lang/translate_url_provider.h"

#include "base/debug_log.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonParseError>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace Ui {
namespace {

[[nodiscard]] bool SkipJsonKey(const QString &key) {
	return (key.compare(u"code"_q, Qt::CaseInsensitive) == 0);
}

[[nodiscard]] QString DetectFromLanguage(const QString &text) {
	// The local recognizer's guess doesn't have to match a language the
	// user's chosen server actually has loaded (e.g. it may return "bg"
	// for text a self-hosted LibreTranslate instance never installed),
	// which the server then rejects outright. Custom providers we support
	// (LibreTranslate, Lingva) both handle server-side auto-detection
	// reliably, so let the server decide instead of trusting a client-side
	// guess it might not be able to satisfy.
	return u"auto"_q;
}

[[nodiscard]] QString JsonValueToText(const QJsonValue &v) {
	switch (v.type()) {
	case QJsonValue::Null: return u"null"_q;
	case QJsonValue::Bool: return v.toBool()
		? u"true"_q
		: u"false"_q;
	case QJsonValue::Double: return QString::number(v.toDouble(), 'g', 15);
	case QJsonValue::String: return v.toString().trimmed();
	case QJsonValue::Array: return QString::fromUtf8(
		QJsonDocument(v.toArray()).toJson(QJsonDocument::Compact));
	case QJsonValue::Object: return QString::fromUtf8(
		QJsonDocument(v.toObject()).toJson(QJsonDocument::Compact));
	case QJsonValue::Undefined: return QString();
	}
	return QString();
}

[[nodiscard]] std::optional<QString> ParseSegmentedArrayResponse(
		const QJsonDocument &parsed) {
	if (!parsed.isArray()) {
		return std::nullopt;
	}
	const auto root = parsed.array();
	if (root.isEmpty() || !root[0].isArray()) {
		return std::nullopt;
	}
	const auto segments = root[0].toArray();
	auto translated = QString();
	for (const auto &segmentValue : segments) {
		if (!segmentValue.isArray()) {
			return std::nullopt;
		}
		const auto segment = segmentValue.toArray();
		if (segment.isEmpty()) {
			return std::nullopt;
		}
		if (!segment[0].isString()) {
			return std::nullopt;
		}
		translated += segment[0].toString();
	}
	if (translated.trimmed().isEmpty()) {
		return std::nullopt;
	}
	return translated;
}

struct JsonLine {
	QString name;
	QString value;
	int length = 0;
};

void CollectJsonLines(
		const QString &name,
		const QJsonValue &value,
		std::vector<JsonLine> &lines) {
	if (value.isObject()) {
		const auto object = value.toObject();
		for (auto i = object.constBegin(); i != object.constEnd(); ++i) {
			if (SkipJsonKey(i.key())) {
				continue;
			}
			CollectJsonLines(
				name.isEmpty() ? i.key() : (name + '.' + i.key()),
				i.value(),
				lines);
		}
		return;
	}
	if (value.isArray()) {
		const auto array = value.toArray();
		for (auto i = 0; i != array.size(); ++i) {
			CollectJsonLines(
				u"%1[%2]"_q.arg(name).arg(i),
				array.at(i),
				lines);
		}
		return;
	}
	const auto text = JsonValueToText(value);
	if (text.isEmpty()) {
		return;
	}
	lines.push_back(JsonLine{
		.name = name,
		.value = text,
		.length = int(text.size()),
	});
}

[[nodiscard]] std::optional<QString> ParseLibreTranslateResponse(
		const QJsonDocument &parsed) {
	if (!parsed.isObject()) {
		return std::nullopt;
	}
	const auto object = parsed.object();
	const auto i = object.constFind(u"translatedText"_q);
	if (i == object.constEnd() || !i->isString()) {
		return std::nullopt;
	}
	return i->toString();
}

[[nodiscard]] std::optional<QString> FormatJsonResponse(
		const QByteArray &body) {
	auto error = QJsonParseError();
	const auto parsed = QJsonDocument::fromJson(body, &error);
	if (error.error != QJsonParseError::NoError) {
		return std::nullopt;
	}
	if (const auto direct = ParseLibreTranslateResponse(parsed)) {
		return direct;
	}
	if (const auto parsedArray = ParseSegmentedArrayResponse(parsed)) {
		return parsedArray;
	}
	auto lines = std::vector<JsonLine>();
	if (parsed.isObject()) {
		const auto object = parsed.object();
		for (auto i = object.constBegin(); i != object.constEnd(); ++i) {
			if (SkipJsonKey(i.key())) {
				continue;
			}
			CollectJsonLines(i.key(), i.value(), lines);
		}
	} else if (parsed.isArray()) {
		const auto array = parsed.array();
		for (auto i = 0; i != array.size(); ++i) {
			CollectJsonLines(u"[%1]"_q.arg(i), array.at(i), lines);
		}
	}
	if (lines.empty()) {
		return QString::fromUtf8(body);
	}
	ranges::sort(lines, [](const JsonLine &a, const JsonLine &b) {
		return (a.length != b.length)
			? (a.length > b.length)
			: (a.name < b.name);
	});
	auto result = QString();
	result.reserve(lines.size() * 16);
	for (auto i = 0; i != int(lines.size()); ++i) {
		const auto &line = lines[i];
		if (!line.name.isEmpty()) {
			result += line.name;
			result += '\n';
		}
		result += line.value;
		if (i + 1 != int(lines.size())) {
			result += "\n\n";
		}
	}
	return result;
}

class UrlTranslateProvider final : public TranslateProvider {
public:
	explicit UrlTranslateProvider(QString urlTemplate)
	: _urlTemplate(std::move(urlTemplate)) {
		// A custom translate server (often localhost or a LAN address) must
		// always be reached directly: routing it through whatever MTProto
		// proxy the user has configured for reaching Telegram's own servers
		// would make it unreachable and isn't what the user intends.
		_network.setProxy(QNetworkProxy::NoProxy);
	}

	[[nodiscard]] bool supportsMessageId() const override {
		return false;
	}

	void request(
			TranslateProviderRequest request,
			LanguageId to,
			Fn<void(TranslateProviderResult)> done) override {
		if (request.text.text.isEmpty()) {
			done(TranslateProviderResult{
				.error = TranslateProviderError::Unknown,
			});
			return;
		}
		const auto from = DetectFromLanguage(request.text.text);
		const auto toCode = to.twoLetterCode();
		if (_urlTemplate.contains(u"%q"_q)) {
			requestGetTemplate(request.text.text, from, toCode, done);
		} else {
			requestLibreTranslatePost(request.text.text, from, toCode, done);
		}
	}

private:
	void requestGetTemplate(
			const QString &text,
			const QString &from,
			const QString &toCode,
			const Fn<void(TranslateProviderResult)> &done) {
		auto url = _urlTemplate;
		url.replace(
			u"%q"_q,
			QString::fromLatin1(
				QUrl::toPercentEncoding(text.toHtmlEscaped())));
		url.replace(
			u"%f"_q,
			QString::fromLatin1(QUrl::toPercentEncoding(from)));
		url.replace(
			u"%t"_q,
			QString::fromLatin1(QUrl::toPercentEncoding(toCode)));
		const auto requestUrl = QUrl(url);
		if (!requestUrl.isValid()) {
			LOG(("Translate Error: Invalid GET template URL '%1'.").arg(url));
			done(TranslateProviderResult{
				.error = TranslateProviderError::Unknown,
			});
			return;
		}
		send(QNetworkRequest(requestUrl), QByteArray(), done);
	}

	void requestLibreTranslatePost(
			const QString &text,
			const QString &from,
			const QString &toCode,
			const Fn<void(TranslateProviderResult)> &done) {
		auto base = _urlTemplate.trimmed();
		while (base.endsWith('/')) {
			base.chop(1);
		}
		if (!base.endsWith(u"/translate"_q)) {
			base += u"/translate"_q;
		}
		const auto requestUrl = QUrl(base);
		if (!requestUrl.isValid()) {
			LOG(("Translate Error: Invalid LibreTranslate URL '%1'.").arg(base));
			done(TranslateProviderResult{
				.error = TranslateProviderError::Unknown,
			});
			return;
		}
		auto body = QJsonObject();
		body["q"] = text;
		body["source"] = from;
		body["target"] = toCode;
		body["format"] = u"text"_q;
		auto networkRequest = QNetworkRequest(requestUrl);
		networkRequest.setHeader(
			QNetworkRequest::ContentTypeHeader,
			u"application/json"_q);
		send(
			networkRequest,
			QJsonDocument(body).toJson(QJsonDocument::Compact),
			done);
	}

	void send(
			QNetworkRequest networkRequest,
			QByteArray postBody,
			const Fn<void(TranslateProviderResult)> &done) {
		const auto url = networkRequest.url().toString();
		const auto reply = postBody.isEmpty()
			? _network.get(networkRequest)
			: _network.post(networkRequest, postBody);
		QObject::connect(reply, &QNetworkReply::finished, [=] {
			auto result = TranslateProviderResult();
			if (reply->error() != QNetworkReply::NoError) {
				const auto status = reply->attribute(
					QNetworkRequest::HttpStatusCodeAttribute).toInt();
				LOG(("Translate Error: Request to '%1' failed (%2), "
					"HTTP status %3: %4"
					).arg(url
					).arg(int(reply->error())
					).arg(status
					).arg(reply->errorString()));
				result.error = TranslateProviderError::Unknown;
			} else {
				const auto body = reply->readAll();
				const auto formatted = FormatJsonResponse(body);
				if (!formatted) {
					DEBUG_LOG(("Translate Warning: Could not parse response "
						"from '%1': %2").arg(url, QString::fromUtf8(body)));
				}
				result.text = TextWithEntities{
					formatted.value_or(QString::fromUtf8(body)),
				};
			}
			done(std::move(result));
			reply->deleteLater();
		});
	}

	const QString _urlTemplate;
	QNetworkAccessManager _network;

};

} // namespace

std::unique_ptr<TranslateProvider> CreateUrlTranslateProvider(
		QString urlTemplate) {
	return std::make_unique<UrlTranslateProvider>(std::move(urlTemplate));
}

} // namespace Ui
