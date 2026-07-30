import { ExternalLink, Send } from 'lucide-react'

export default function Footer({ site, footer }) {
  return (
    <footer className="footer">
      <div className="container">
        <div className="footer-inner">

          <div className="footer-brand">
            <span className="footer-brand-mark">
              <Send size={11} strokeWidth={2.5} />
            </span>
            {site.name}
          </div>

          <div className="footer-links">
            {footer.links.map(l => (
              <a
                key={l.label}
                className="footer-link"
                href={l.href}
                target="_blank"
                rel="noopener noreferrer"
              >
                {l.label}
                <ExternalLink size={10} />
              </a>
            ))}
          </div>

          <p className="footer-notice">{footer.notice}</p>

        </div>
      </div>
    </footer>
  )
}
