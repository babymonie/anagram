import { Icon } from './IconMap'

export default function FeatureCard({ feature }) {
  return (
    <div className="fcard">
      <div className="fcard-top">
        <span className="fcard-icon">
          <Icon name={feature.icon} size={15} />
        </span>
        {feature.badge && (
          <span className={`badge badge-${feature.badgeVariant ?? 'blue'}`}>
            {feature.badge}
          </span>
        )}
      </div>
      <div className="fcard-title">{feature.title}</div>
      <p className="fcard-desc">{feature.description}</p>
    </div>
  )
}
