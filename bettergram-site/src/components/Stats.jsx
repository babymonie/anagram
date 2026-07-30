export default function Stats({ stats }) {
  return (
    <div className="stats">
      <div className="container">
        <div className="stats-inner">
          {stats.map(s => (
            <div className="stat-item" key={s.label}>
              <div className="stat-value">{s.value}</div>
              <div className="stat-label">{s.label}</div>
            </div>
          ))}
        </div>
      </div>
    </div>
  )
}
