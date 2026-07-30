import { Menu, Send } from 'lucide-react'
import { Icon } from './IconMap'

const links = [
  { href: '#setup',    label: 'Setup' },
  { href: '#features', label: 'Features' },
  { href: '#hub',      label: 'Hubs' },
  { href: '#install',  label: 'Build' },
  { href: '#faq',      label: 'FAQ' },
]

export default function Navbar({ site }) {
  return (
    <nav className="navbar">
      <div className="container navbar-inner">
        <a className="navbar-logo" href="#top">
          <span className="navbar-logo-mark">
            <Send size={14} strokeWidth={2.5} />
          </span>
          {site.name}
        </a>

        <div className="navbar-nav">
          {links.map(l => (
            <a key={l.href} className="navbar-link" href={l.href}>{l.label}</a>
          ))}
        </div>

        <div className="navbar-actions">
          <a className="navbar-gh" href={site.github} target="_blank" rel="noopener">
            <Icon name="Github" size={14} />
            GitHub
          </a>
          <button className="navbar-mobile-toggle">
            <Menu size={18} />
          </button>
        </div>
      </div>
    </nav>
  )
}
