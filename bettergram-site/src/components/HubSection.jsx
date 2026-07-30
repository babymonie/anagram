export default function HubSection({ hub }) {
  return (
    <section className="section-alt section-divider" id="hub">
      <div className="container">
        <div className="hub-grid">

          <div>
            <div className="sh">
              <span className="sh-label">Federation</span>
              <h2 className="sh-title">{hub.headline}</h2>
              <p className="sh-desc">{hub.description}</p>
            </div>

            <div className="hub-points">
              {hub.points.map(p => (
                <div className="hub-pt" key={p.title}>
                  <span className="hub-pt-bullet" />
                  <div>
                    <div className="hub-pt-title">{p.title}</div>
                    <p className="hub-pt-desc">{p.description}</p>
                  </div>
                </div>
              ))}
            </div>
          </div>

          <div>
            <div className="hub-panel">
              <p className="hub-panel-label">Default local hub</p>
              <div className="hub-default">
                <div>
                  <span className="hub-default-k">Hub URL</span>
                  <code>{hub.defaultUrl}</code>
                </div>
                <div>
                  <span className="hub-default-k">Name</span>
                  <strong>{hub.defaultName}</strong>
                </div>
                <div>
                  <span className="hub-default-k">Admin token</span>
                  <code>{hub.adminToken}</code>
                </div>
              </div>

              <p className="hub-panel-label">What hubs unlock</p>
              <div className="hub-cap-list">
                {hub.capabilities.map(cap => (
                  <div className="hub-cap-item" key={cap}>{cap}</div>
                ))}
              </div>

              <div className="hub-terminal">
                <div className="hub-terminal-bar">
                  <span className="hub-term-dot" />
                  <span className="hub-term-dot" />
                  <span className="hub-term-dot" />
                  <span className="hub-term-label">zsh</span>
                </div>
                <pre className="hub-cmd">
                  <span className="cmd-dim"># Run the local hub from this repo{'\n'}</span>
                  {'cd bettergram-hub\n'}
                  {'docker compose up -d --build\n'}
                  <span className="cmd-dim">{'\n# Verify it works\n'}</span>
                  {'curl http://localhost:8080/health\n'}
                  {'curl http://localhost:8080/info'}
                </pre>
              </div>
            </div>
          </div>

        </div>
      </div>
    </section>
  )
}
