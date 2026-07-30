#pragma once

#include "base/weak_ptr.h"
#include "base/timer.h"

namespace Calls {
class GroupCall;
} // namespace Calls

namespace BetterGram {

// Frameless always-on-top floating widget shown during an active group call.
// Provides mute toggle, hang-up, and expand-to-full-panel controls.
class BgCallMini final : public QWidget {
	Q_OBJECT
public:
	explicit BgCallMini(
		not_null<Calls::GroupCall*> call,
		Fn<void()> onExpand);
	~BgCallMini();

protected:
	void paintEvent(QPaintEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseDoubleClickEvent(QMouseEvent *e) override;

private:
	void subscribeToCall();
	void toggleMute();
	void hangup();

	[[nodiscard]] QRect muteRect() const;
	[[nodiscard]] QRect expandRect() const;
	[[nodiscard]] QRect hangupRect() const;

	static constexpr int kWidth = 264;
	static constexpr int kHeight = 52;
	static constexpr int kRadius = 10;
	static constexpr int kButtonSize = 28;
	static constexpr int kMargin = 10;

	base::weak_ptr<Calls::GroupCall> _call;
	Fn<void()> _onExpand;

	QString _peerName;
	bool _muted = true;
	float _speakLevel = 0.f;

	QPoint _dragStart;
	QPoint _posAtDrag;
	bool _dragging = false;

	base::Timer _levelDecay;
	rpl::lifetime _lifetime;
};

} // namespace BetterGram
