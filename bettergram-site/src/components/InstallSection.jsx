import { useState } from 'react'
import { Check, ChevronDown, Copy } from 'lucide-react'

function CodeBlock({ code }) {
  const [copied, setCopied] = useState(false)

  function copy() {
    navigator.clipboard.writeText(code)
    setCopied(true)
    setTimeout(() => setCopied(false), 1800)
  }

  const lines = code.split('\n').map(line =>
    line.startsWith('#') ? { type: 'comment', text: line } : { type: 'cmd', text: line }
  )

  return (
    <div className="code-block">
      {lines.map(({ type, text }, i) => (
        <div key={`${i}-${text.slice(0, 8)}`}>
          {type === 'comment'
            ? <span className="cmd-dim">{text}{'\n'}</span>
            : <span><span className="cb-prompt">$ </span>{text}{'\n'}</span>
          }
        </div>
      ))}
      <button className={`code-copy${copied ? ' copied' : ''}`} onClick={copy}>
        {copied ? <Check size={11} /> : <Copy size={11} />}
        {copied ? 'Copied' : 'Copy'}
      </button>
    </div>
  )
}

export default function InstallSection({ steps }) {
  const [open, setOpen] = useState(0)

  return (
    <section className="section section-divider" id="install">
      <div className="container">
        <div className="sh sh-center">
          <span className="sh-label">Build from source</span>
          <h2 className="sh-title">Install or build BetterGram</h2>
          <p className="sh-desc">
            Build it like Telegram Desktop, then launch the app and sign in
            with your normal Telegram account.
          </p>
        </div>

        <div className="install-steps">
          {steps.map((step, i) => (
            <div
              key={step.step}
              className={`istep${open === i ? ' open' : ''}`}
            >
              <button
                type="button"
                className="istep-head"
                onClick={() => setOpen(open === i ? -1 : i)}
              >
                <span className="istep-num">{step.step}</span>
                <span className="istep-title">{step.title}</span>
                <ChevronDown size={15} className="istep-chevron" />
              </button>
              {open === i && (
                <div className="istep-body">
                  <CodeBlock code={step.code} />
                </div>
              )}
            </div>
          ))}
        </div>
      </div>
    </section>
  )
}
