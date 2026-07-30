#include "bettergram/bg_call_mini.h"

#include "calls/group/calls_group_call.h"
#include "data/data_peer.h"
#include "core/application.h"
#include "calls/calls_instance.h"

#include <QPainter>
#include <QMouseEvent>
#include <QScreen>
#include <QGuiApplication>

namespace BetterGram {

BgCallMini::BgCallMini(
	not_null<Calls::GroupCall*> call,
	Fn<void()> onExpand)
: QWidget(
	nullptr,
	Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
, _call(call)
, _onExpand(std::move(onExpand))
, _levelDecay([=] {
	_speakLevel = std::max(0.f, _speakLevel - 0.12f);
	update();
	if (_speakLevel <= 0.f) {
		_levelDecay.cancel();
	}
}) {
	setAttribute(Qt::WA_TranslucentBackground);
	setFixedSize(kWidth, kHeight);

	if (const auto screen = QGuiApplication::primaryScreen()) {
		const auto g = screen->availableGeometry();
		move(g.right() - kWidth - 20, g.bottom() - kHeight - 20);
	}

	subscribeToCall();
	show();
	raise();
}

BgCallMini::~BgCallMini() = default;

void BgCallMini::subscribeToCall() {
	const auto group = _call.get();
	if (!group) return;

	_peerName = group->peer()->name();
	_muted = (group->muted() != Calls::MuteState::Active);

	group->mutedValue(
	) | rpl::on_next([=](Calls::MuteState state) {
		_muted = (state != Calls::MuteState::Active);
		update();
	}, _lifetime);

	group->levelUpdates(
	) | rpl::on_next([=](const Calls::LevelUpdate &upd) {
		if (upd.value > _speakLevel) {
			_speakLevel = std::min(1.f, upd.value * 2.5f);
		}
		if (!_levelDecay.isActive()) {
			_levelDecay.callEach(40);
		}
		update();
	}, _lifetime);

	group->stateValue(
	) | rpl::on_next([=](Calls::GroupCall::State state) {
		using State = Calls::GroupCall::State;
		if (state == State::HangingUp || state == State::Ended) {
			close();
		}
	}, _lifetime);
}

void BgCallMini::paintEvent(QPaintEvent *) {
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);

	// Outer glow ring when speaking
	if (_speakLevel > 0.05f) {
		const auto alpha = static_cast<int>(_speakLevel * 60);
		p.setPen(QPen(QColor(80, 220, 120, alpha), 3));
		p.setBrush(Qt::NoBrush);
		p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), kRadius + 1, kRadius + 1);
	}

	// Background: green when active, dark when muted
	const QColor bg = _muted
		? QColor(48, 48, 60, 230)
		: QColor(30, 150, 90, 230);
	p.setPen(Qt::NoPen);
	p.setBrush(bg);
	p.drawRoundedRect(rect(), kRadius, kRadius);

	// Peer name
	p.setPen(Qt::white);
	QFont f = font();
	f.setPixelSize(13);
	f.setBold(true);
	p.setFont(f);

	const int textLeft = kMargin + kButtonSize + kMargin;
	const int textRight = kWidth - kMargin - kButtonSize - kMargin - kButtonSize - kMargin;
	const QRect textRect(textLeft, 0, textRight - textLeft, kHeight);
	const auto elided = p.fontMetrics().elidedText(
		_peerName, Qt::ElideRight, textRect.width());
	p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elided);

	// Draw a small button circle helper
	const auto drawBtn = [&](QRect r, QColor fill, const QString &label) {
		p.setBrush(fill);
		p.setPen(Qt::NoPen);
		p.drawEllipse(r);
		p.setPen(Qt::white);
		QFont sf = font();
		sf.setPixelSize(11);
		p.setFont(sf);
		p.drawText(r, Qt::AlignCenter, label);
	};

	// Mute button (left)
	drawBtn(muteRect(),
		_muted ? QColor(200, 60, 60, 200) : QColor(60, 180, 100, 200),
		_muted ? u"M"_q : u"m"_q);

	// Expand button
	drawBtn(expandRect(), QColor(80, 80, 120, 200), u"^"_q);

	// Hang-up button (right)
	drawBtn(hangupRect(), QColor(200, 60, 60, 200), u"x"_q);
}

QRect BgCallMini::muteRect() const {
	return QRect(kMargin, (kHeight - kButtonSize) / 2, kButtonSize, kButtonSize);
}

QRect BgCallMini::expandRect() const {
	return QRect(
		kWidth - kMargin - kButtonSize * 2 - kMargin,
		(kHeight - kButtonSize) / 2,
		kButtonSize,
		kButtonSize);
}

QRect BgCallMini::hangupRect() const {
	return QRect(
		kWidth - kMargin - kButtonSize,
		(kHeight - kButtonSize) / 2,
		kButtonSize,
		kButtonSize);
}

void BgCallMini::toggleMute() {
	if (const auto group = _call.get()) {
		if (group->mutedByAdmin()) return;
		group->setMuted(
			group->muted() == Calls::MuteState::Muted
				? Calls::MuteState::Active
				: Calls::MuteState::Muted);
	}
}

void BgCallMini::hangup() {
	if (const auto group = _call.get()) {
		group->hangup();
	}
	close();
}

void BgCallMini::mousePressEvent(QMouseEvent *e) {
	if (e->button() != Qt::LeftButton) return;

	if (muteRect().contains(e->pos())) {
		toggleMute();
	} else if (hangupRect().contains(e->pos())) {
		hangup();
	} else if (expandRect().contains(e->pos())) {
		if (_onExpand) _onExpand();
		close();
	} else {
		_dragging = true;
		_dragStart = e->globalPos();
		_posAtDrag = pos();
	}
}

void BgCallMini::mouseMoveEvent(QMouseEvent *e) {
	if (_dragging) {
		move(_posAtDrag + (e->globalPos() - _dragStart));
	}
}

void BgCallMini::mouseReleaseEvent(QMouseEvent *) {
	_dragging = false;
}

void BgCallMini::mouseDoubleClickEvent(QMouseEvent *) {
	if (_onExpand) _onExpand();
	close();
}

} // namespace BetterGram
