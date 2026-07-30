import {
  CheckCircle2,
  KeyRound,
  Link,
  MessageSquareText,
  Phone,
  Settings,
  ShieldCheck,
} from 'lucide-react'

const loginSteps = [
  {
    icon: Phone,
    title: 'Enter your number',
    text: 'Launch BetterGram and type the same phone number you use for Telegram. Keep the country code selected correctly.',
  },
  {
    icon: MessageSquareText,
    title: 'Approve the code',
    text: 'Telegram sends the login code to your Telegram app first. If needed, wait for SMS or phone-call fallback.',
  },
  {
    icon: KeyRound,
    title: 'Finish 2FA',
    text: 'If your account has a cloud password, enter it. BetterGram uses the normal Telegram Desktop login flow.',
  },
  {
    icon: Settings,
    title: 'Open BetterGram settings',
    text: 'Go to Settings > BetterGram. This is where ghost messages, privacy cleanup, hubs, AI tools, and themes live.',
  },
]

const usageSteps = [
  'Turn on Ghost Messages to keep remotely deleted incoming messages visible in grey.',
  'Enable privacy cleanup such as sponsored-message hiding, story hiding, premium badge hiding, and link de-tracking.',
  'Add optional services only when you need them: LibreTranslate/Lingva for translation, Whisper HTTP for voice transcription, and OpenAI-compatible APIs for AI compose.',
  'Use right-click menus in chats for BetterGram actions like contact notes, bulk media download, peer IDs, and custom sounds.',
]

export default function TutorialSection() {
  return (
    <section className="section section-divider" id="setup">
      <div className="container">
        <div className="tutorial-grid">
          <div className="tutorial-main">
            <div className="sh">
              <span className="sh-label">Start here</span>
              <h2 className="sh-title">First login and setup</h2>
              <p className="sh-desc">
                BetterGram signs in like Telegram Desktop. After login, the extra
                controls are grouped under Settings &gt; BetterGram.
              </p>
            </div>

            <div className="tutorial-steps">
              {loginSteps.map((step, index) => {
                const Icon = step.icon
                return (
                  <div className="tutorial-step" key={step.title}>
                    <div className="tutorial-num">{index + 1}</div>
                    <div className="tutorial-icon"><Icon size={17} /></div>
                    <div>
                      <h3>{step.title}</h3>
                      <p>{step.text}</p>
                    </div>
                  </div>
                )
              })}
            </div>
          </div>

          <aside className="tutorial-panel">
            <div className="tutorial-panel-head">
              <ShieldCheck size={18} />
              <span>How to use BetterGram</span>
            </div>
            <div className="tutorial-checks">
              {usageSteps.map(step => (
                <div className="tutorial-check" key={step}>
                  <CheckCircle2 size={15} />
                  <p>{step}</p>
                </div>
              ))}
            </div>
            <div className="tutorial-hub-note">
              <Link size={15} />
              <p>
                Hubs are optional. Use BetterGram without a hub for local client
                features, or paste http://localhost:8080 as the default local
                hub to unlock shared badges, profiles, reactions, announcements,
                and hub themes.
              </p>
            </div>
          </aside>
        </div>
      </div>
    </section>
  )
}
