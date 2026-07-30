import { useState } from 'react'
import { ChevronDown } from 'lucide-react'

export default function FAQ({ faq }) {
  const [open, setOpen] = useState(null)

  return (
    <section className="section section-divider" id="faq">
      <div className="container">
        <div className="sh sh-center">
          <span className="sh-label">Questions</span>
          <h2 className="sh-title">FAQ</h2>
        </div>

        <div className="faq-list">
          {faq.map((item, i) => (
            <div key={item.q} className={`faq-item${open === i ? ' open' : ''}`}>
              <button
                type="button"
                className="faq-q"
                onClick={() => setOpen(open === i ? null : i)}
              >
                {item.q}
                <ChevronDown size={15} className="faq-chevron" />
              </button>
              {open === i && (
                <p className="faq-a">{item.a}</p>
              )}
            </div>
          ))}
        </div>
      </div>
    </section>
  )
}
