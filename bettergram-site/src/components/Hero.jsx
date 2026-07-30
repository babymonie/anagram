import { ArrowDown } from 'lucide-react'
import { Icon } from './IconMap'
import AppPreview from './AppPreview'

export default function Hero({ site, stats }) {
  return (
    <section className="hero" id="top">
      <div className="container">
        <div className="hero-grid">

          <div className="hero-content">
            <p className="hero-eyebrow">
              Open source · GPLv3 · Built on <span>Telegram Desktop</span>
            </p>

            <h1 className="hero-h1">
              Set up BetterGram<br />
              without <em>guessing</em>.
            </h1>

            <p className="hero-desc">
              {site.description}
            </p>

            <div className="hero-actions">
              <a className="btn btn-primary" href="#setup">
                <ArrowDown size={14} strokeWidth={2.5} />
                Start Setup
              </a>
              <a className="btn btn-ghost" href="#hub">
                <Icon name="Globe" size={14} />
                Hub Tutorial
              </a>
            </div>

            <div className="hero-stats">
              {stats.map((s, i) => (
                <div key={s.label} style={{ display: 'contents' }}>
                  {i > 0 && <span className="hero-stat-divider" />}
                  <div className="hero-stat">
                    <strong>{s.value}</strong>
                    {s.label}
                  </div>
                </div>
              ))}
            </div>
          </div>

          <AppPreview />

        </div>
      </div>
    </section>
  )
}
