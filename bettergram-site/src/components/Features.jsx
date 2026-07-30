import { useMemo, useState } from 'react'
import { Search, X } from 'lucide-react'
import { Icon } from './IconMap'
import FeatureCard from './FeatureCard'

export default function Features({ categories, features }) {
  const [activeCat, setActiveCat] = useState('all')
  const [query, setQuery] = useState('')

  const filtered = useMemo(() => {
    const q = query.toLowerCase().trim()
    return features.filter(feature => {
      const catMatch = activeCat === 'all' || feature.category === activeCat
      const searchMatch = !q
        || feature.title.toLowerCase().includes(q)
        || feature.description.toLowerCase().includes(q)
      return catMatch && searchMatch
    })
  }, [features, activeCat, query])

  return (
    <section className="section section-divider" id="features">
      <div className="container">
        <div className="features-header-row">
          <div className="sh">
            <span className="sh-label">Everything included</span>
            <h2 className="sh-title">What to turn on</h2>
            <p className="sh-desc">
              Every feature ships in the binary. Enable or configure them in
              Settings &gt; BetterGram.
            </p>
          </div>

          <div className="search-wrap">
            <span className="search-icon"><Search size={13} /></span>
            <input
              className="search-input"
              placeholder="Search..."
              value={query}
              onChange={event => setQuery(event.target.value)}
            />
            {query && (
              <button className="search-clear" onClick={() => setQuery('')}>
                <X size={12} />
              </button>
            )}
          </div>
        </div>

        <div className="cat-tabs">
          {categories.map(cat => (
            <button
              key={cat.id}
              className={`cat-tab${activeCat === cat.id ? ' active' : ''}`}
              onClick={() => setActiveCat(cat.id)}
            >
              <Icon name={cat.icon} size={12} />
              {cat.label}
            </button>
          ))}
        </div>

        <p className="features-meta">
          {filtered.length} feature{filtered.length === 1 ? '' : 's'}
          {activeCat === 'all' && !query ? ' total' : ' matched'}
        </p>

        <div className="features-grid">
          {filtered.length > 0 ? (
            filtered.map(feature => <FeatureCard key={feature.id} feature={feature} />)
          ) : (
            <div className="features-empty">
              <Search size={28} />
              <p>No features match "{query}"</p>
            </div>
          )}
        </div>
      </div>
    </section>
  )
}
