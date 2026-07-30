/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtGui/QColor>
#include <QString>
#include <QVector>

namespace BetterGram {

struct BgPalette {
	QString name;
	QColor bg;
	QColor surface;
	QColor card;
	QColor msgIn;
	QColor msgOut;
	QColor accent;
	QColor text;
	QColor textSub;
};

[[nodiscard]] const QVector<BgPalette> &BundledThemes();
[[nodiscard]] QByteArray DerivePalette(const BgPalette &p);
void ApplyTheme(const BgPalette &p);
void ApplyBundledTheme(const QString &name);
void RestoreTheme();

} // namespace BetterGram
