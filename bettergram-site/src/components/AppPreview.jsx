export default function AppPreview() {
  return (
    <div className="ap-window" aria-hidden="true">
      <div className="ap-titlebar">
        <div className="ap-dots">
          <span className="ap-dot ap-dot-r" />
          <span className="ap-dot ap-dot-y" />
          <span className="ap-dot ap-dot-g" />
        </div>
        <span className="ap-title">BetterGram first run</span>
        <span style={{ width: 42 }} />
      </div>

      <div className="ap-chat">
        <div className="ap-msg-in">
          <div className="ap-avatar">1</div>
          <div className="ap-msg-body">
            <div className="ap-sender">Login</div>
            <div className="ap-bubble">Enter your phone number, confirm the code, then enter your 2FA password if Telegram asks.</div>
            <div className="ap-time">Step 1</div>
          </div>
        </div>

        <div className="ap-msg-in">
          <div className="ap-avatar">2</div>
          <div className="ap-ghost-wrap">
            <div className="ap-sender">Ghost Messages</div>
            <div className="ap-bubble ap-ghost-bubble">
              Deleted messages stay readable, but greyed out.
            </div>
            <div className="ap-time" style={{ opacity: 0.35 }}>Enabled</div>
          </div>
        </div>

        <div className="ap-msg-out">
          <div className="ap-bubble ap-bubble-out">Open Settings, choose BetterGram, then connect your hub URL if you use one.</div>
          <div className="ap-time ap-time-out">Step 2</div>
        </div>
      </div>

      <div className="ap-feature-label">
        <svg width="10" height="10" viewBox="0 0 10 10" fill="currentColor">
          <circle cx="5" cy="5" r="4" />
        </svg>
        Setup guide: login, enable features, connect hubs
      </div>

      <div style={{ height: 12 }} />

      <div className="ap-bar">
        <div className="ap-input">Message</div>
        <div className="ap-send">&gt;</div>
      </div>
    </div>
  )
}
