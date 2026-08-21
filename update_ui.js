const fs = require('fs');
const path = require('path');
const srcDir = path.join(__dirname, 'web', 'frontend', 'src');

const files = {
  'App.css': `
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&family=JetBrains+Mono:wght@400;500&display=swap');

:root {
  --bg-base: #000000;
  --bg-surface: #09090b;
  --bg-surface-hover: #18181b;
  --text-primary: #ffffff;
  --text-secondary: #a1a1aa;
  --border-light: #27272a;
  --border-focus: #3f3f46;
  --accent-blue: #3b82f6;
  --accent-purple: #8b5cf6;
  --danger: #ef4444;
  --success: #10b981;
}

* { box-sizing: border-box; margin: 0; padding: 0; }

body {
  background-color: var(--bg-base);
  color: var(--text-primary);
  font-family: 'Inter', -apple-system, sans-serif;
  -webkit-font-smoothing: antialiased;
  min-height: 100vh;
  background-image: 
    radial-gradient(circle at 15% 50%, rgba(59, 130, 246, 0.08), transparent 25%),
    radial-gradient(circle at 85% 30%, rgba(139, 92, 246, 0.08), transparent 25%);
}

.app { display: flex; flex-direction: column; min-height: 100vh; }

.main-content {
  flex: 1; max-width: 1200px; width: 100%; margin: 0 auto; padding: 40px 20px;
  animation: fadeIn 0.5s cubic-bezier(0.16, 1, 0.3, 1);
}

@keyframes fadeIn {
  from { opacity: 0; transform: translateY(15px); }
  to { opacity: 1; transform: translateY(0); }
}

.card {
  background: rgba(9, 9, 11, 0.7);
  backdrop-filter: blur(16px);
  -webkit-backdrop-filter: blur(16px);
  border: 1px solid var(--border-light);
  border-radius: 16px;
  padding: 30px;
  box-shadow: 0 12px 40px rgba(0, 0, 0, 0.5);
  transition: transform 0.3s ease, border-color 0.3s ease;
}

.card:hover { border-color: var(--border-focus); }
`,

  'components/Header.css': `
.header {
  border-bottom: 1px solid var(--border-light);
  background: rgba(0, 0, 0, 0.6);
  backdrop-filter: blur(16px);
  -webkit-backdrop-filter: blur(16px);
  position: sticky; top: 0; z-index: 100;
}

.header-content {
  max-width: 1200px; margin: 0 auto; height: 70px;
  display: flex; justify-content: space-between; align-items: center; padding: 0 20px;
}

.logo h1 {
  font-size: 1.5rem; font-weight: 700; letter-spacing: -0.04em;
  background: linear-gradient(135deg, #fff 0%, #a1a1aa 100%);
  -webkit-background-clip: text; color: transparent;
  display: flex; align-items: center; gap: 10px;
}

.logo-icon {
  background: linear-gradient(135deg, var(--accent-blue), var(--accent-purple));
  width: 24px; height: 24px; border-radius: 6px; display: inline-block;
  box-shadow: 0 0 15px rgba(59, 130, 246, 0.5);
}

.nav-btn {
  background: transparent; border: none; color: var(--text-secondary);
  padding: 8px 16px; border-radius: 8px; font-size: 0.95rem; font-weight: 500; cursor: pointer;
  transition: all 0.2s ease;
}

.nav-btn:hover { color: var(--text-primary); background: var(--bg-surface-hover); }
.nav-btn.active { color: var(--text-primary); background: var(--bg-surface); border: 1px solid var(--border-light); }
`,

  'pages/Upload.css': `
.upload-page { max-width: 800px; margin: 0 auto; display: flex; flex-direction: column; gap: 30px; }

.upload-page h2 {
  font-size: 2.5rem; text-align: center; margin-bottom: 10px; font-weight: 700; letter-spacing: -0.04em;
  background: linear-gradient(to right, #fff, #a1a1aa); -webkit-background-clip: text; color: transparent;
}

.drop-zone {
  border: 2px dashed var(--border-focus); border-radius: 20px; padding: 60px 40px;
  text-align: center; background: rgba(9, 9, 11, 0.5); cursor: pointer;
  transition: all 0.3s cubic-bezier(0.16, 1, 0.3, 1);
  display: flex; flex-direction: column; align-items: center; gap: 16px;
}

.drop-zone:hover {
  border-color: var(--accent-blue); background: rgba(59, 130, 246, 0.05); transform: translateY(-4px);
  box-shadow: 0 10px 30px rgba(59, 130, 246, 0.1);
}

.upload-icon { width: 48px; height: 48px; color: var(--accent-blue); opacity: 0.9; }
.drop-zone p:first-of-type { font-size: 1.15rem; font-weight: 500; color: #fff; }
.supported-exts { color: var(--text-secondary); font-size: 0.9rem; }

.file-list ul { list-style: none; display: flex; flex-direction: column; gap: 10px; margin-top: 15px; }
.file-list li {
  display: flex; justify-content: space-between; align-items: center; padding: 14px 20px;
  background: var(--bg-surface-hover); border-radius: 12px; border: 1px solid var(--border-light);
  font-family: 'JetBrains Mono', monospace; font-size: 0.9rem;
  animation: fadeIn 0.3s ease-out;
}
.file-size { color: var(--text-secondary); background: var(--bg-surface); padding: 4px 10px; border-radius: 12px; font-size: 0.8rem; }

.config-panel { display: flex; flex-direction: column; gap: 28px; }
.config-group { display: flex; flex-direction: column; gap: 14px; }
.config-group label { display: flex; justify-content: space-between; font-weight: 500; color: var(--text-secondary); }
.config-group label span { color: var(--text-primary); font-family: 'JetBrains Mono', monospace; font-weight: 600; }

input[type=range] { -webkit-appearance: none; width: 100%; background: transparent; }
input[type=range]::-webkit-slider-thumb {
  -webkit-appearance: none; height: 20px; width: 20px; border-radius: 50%;
  background: #fff; cursor: pointer; margin-top: -8px;
  box-shadow: 0 0 15px rgba(255,255,255,0.3); transition: transform 0.1s;
}
input[type=range]::-webkit-slider-thumb:hover { transform: scale(1.15); }
input[type=range]::-webkit-slider-runnable-track { width: 100%; height: 4px; cursor: pointer; background: var(--border-focus); border-radius: 2px; }
input[type=range]:focus::-webkit-slider-runnable-track { background: linear-gradient(90deg, var(--accent-blue), var(--accent-purple)); }

.actions { display: flex; justify-content: center; margin-top: 20px; }
.analyze-btn {
  background: linear-gradient(135deg, var(--accent-blue), var(--accent-purple));
  color: white; border: none; padding: 16px 48px; font-size: 1.15rem; font-weight: 600;
  border-radius: 30px; box-shadow: 0 8px 25px rgba(139, 92, 246, 0.4); cursor: pointer; 
  transition: all 0.3s cubic-bezier(0.16, 1, 0.3, 1); letter-spacing: 0.02em;
}
.analyze-btn:hover:not(:disabled) { transform: translateY(-3px) scale(1.02); box-shadow: 0 12px 30px rgba(139, 92, 246, 0.6); }
.analyze-btn:disabled { background: var(--border-light); color: var(--text-secondary); box-shadow: none; cursor: not-allowed; }

.error-msg {
  background: rgba(239, 68, 68, 0.1); color: var(--danger); padding: 16px; border-radius: 12px;
  border: 1px solid rgba(239, 68, 68, 0.2); text-align: center; font-weight: 500;
}
`,

  'pages/Results.css': `
.results-page { display: flex; flex-direction: column; gap: 40px; }
.results-header { display: flex; justify-content: space-between; align-items: flex-end; border-bottom: 1px solid var(--border-light); padding-bottom: 24px; }
.results-header h2 { font-size: 2.2rem; font-weight: 700; letter-spacing: -0.04em; background: linear-gradient(to right, #fff, #a1a1aa); -webkit-background-clip: text; color: transparent; }

.stats-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 24px; }
.stat-card {
  background: linear-gradient(180deg, rgba(24, 24, 27, 0.6) 0%, rgba(9, 9, 11, 0.8) 100%);
  border: 1px solid var(--border-light); border-radius: 16px; padding: 28px 24px;
  display: flex; flex-direction: column; gap: 12px; position: relative; overflow: hidden;
  box-shadow: 0 4px 20px rgba(0,0,0,0.2);
}
.stat-card::before {
  content: ''; position: absolute; top: 0; left: 0; right: 0; height: 2px;
  background: linear-gradient(90deg, transparent, var(--border-focus), transparent);
}
.stat-card:hover::before { background: linear-gradient(90deg, transparent, var(--accent-blue), transparent); }
.stat-label { color: var(--text-secondary); font-size: 0.85rem; font-weight: 600; text-transform: uppercase; letter-spacing: 0.08em; }
.stat-value { font-size: 2.8rem; font-weight: 700; font-family: 'JetBrains Mono', monospace; color: var(--text-primary); letter-spacing: -0.03em; }

.results-content { display: grid; grid-template-columns: 1fr 1fr; gap: 30px; }
@media (max-width: 900px) { .results-content { grid-template-columns: 1fr; } }
.matches-section, .matrix-section, .compare-section { display: flex; flex-direction: column; gap: 20px; }

table { width: 100%; border-collapse: separate; border-spacing: 0; }
th { text-align: left; padding: 16px; color: var(--text-secondary); font-weight: 600; font-size: 0.9rem; border-bottom: 1px solid var(--border-light); letter-spacing: 0.02em; }
td { padding: 18px 16px; border-bottom: 1px solid var(--border-light); font-family: 'JetBrains Mono', monospace; font-size: 0.95rem; color: #e4e4e7; }
tr { transition: background 0.2s, transform 0.2s; cursor: pointer; }
tr:hover { background: rgba(255, 255, 255, 0.03); transform: scale(1.01); border-radius: 8px; }
tr.selected { background: rgba(59, 130, 246, 0.1); border-left: 3px solid var(--accent-blue); }

.sim-badge {
  padding: 6px 14px; border-radius: 20px; font-weight: 600; color: white; display: inline-block;
  min-width: 75px; text-align: center; box-shadow: 0 4px 12px rgba(0,0,0,0.3); text-shadow: 0 1px 2px rgba(0,0,0,0.5);
}
`,

  'components/SimilarityMatrix.css': `
.matrix-wrapper { overflow-x: auto; padding-bottom: 24px; }
.sim-matrix { border-collapse: separate; border-spacing: 6px; }
.sim-matrix th, .sim-matrix td { border: none; }
.rotated-th { height: 140px; position: relative; width: 44px; }
.rotated-th span {
  transform: rotate(-45deg); transform-origin: bottom left; position: absolute;
  bottom: 10px; left: 50%; white-space: nowrap; font-family: 'JetBrains Mono', monospace;
  font-size: 0.85rem; color: var(--text-secondary); font-weight: 500;
}
.row-header {
  font-family: 'JetBrains Mono', monospace; font-size: 0.85rem; color: var(--text-secondary); font-weight: 500;
  text-align: right; padding-right: 20px; max-width: 180px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap;
}
.sim-matrix td:not(.row-header) {
  width: 44px; height: 44px; padding: 0; text-align: center; font-size: 0.8rem; font-weight: 700;
  color: rgba(255, 255, 255, 0.95); border-radius: 8px; cursor: crosshair; transition: transform 0.2s, box-shadow 0.2s;
  box-shadow: inset 0 0 0 1px rgba(0,0,0,0.1); text-shadow: 0 1px 3px rgba(0,0,0,0.8);
}
.sim-matrix td:not(.row-header):hover {
  transform: scale(1.15); box-shadow: 0 8px 20px rgba(0,0,0,0.5); z-index: 10; position: relative;
}
`,

  'components/SourceCompare.css': `
.source-compare { display: grid; grid-template-columns: 1fr 1fr; gap: 24px; }
@media (max-width: 800px) { .source-compare { grid-template-columns: 1fr; } }
.source-pane {
  background: #0d1117; border: 1px solid var(--border-light); border-radius: 12px;
  overflow: hidden; display: flex; flex-direction: column;
  box-shadow: 0 10px 30px rgba(0,0,0,0.4);
}
.source-pane h4 {
  background: #161b22; padding: 14px 20px; font-family: 'Inter', sans-serif;
  font-size: 0.9rem; font-weight: 500; color: #8b949e; border-bottom: 1px solid #30363d;
  display: flex; align-items: center;
}
.source-pane h4::before {
  content: ''; display: inline-block; width: 12px; height: 12px; border-radius: 50%;
  background: #fa5c5c; box-shadow: 20px 0 0 #fdbc40, 40px 0 0 #33c948; margin-right: 50px;
}
.source-pane pre { padding: 20px; overflow-x: auto; flex: 1; max-height: 600px; overflow-y: auto; background: #0d1117; }
.source-pane pre::-webkit-scrollbar { width: 10px; height: 10px; }
.source-pane pre::-webkit-scrollbar-track { background: #0d1117; }
.source-pane pre::-webkit-scrollbar-thumb { background: #30363d; border-radius: 5px; }
.source-pane code {
  font-family: 'JetBrains Mono', Consolas, monospace; font-size: 0.9rem;
  line-height: 1.7; color: #c9d1d9; white-space: pre-wrap;
}
.match-highlight {
  background-color: rgba(248, 81, 73, 0.2); color: #ff7b72; border-radius: 4px;
  border-bottom: 1px dotted #f85149; padding: 2px 0; font-weight: 600;
}
`
};

for (const [relPath, content] of Object.entries(files)) {
  fs.writeFileSync(path.join(srcDir, relPath), content.trim() + '\n');
}
console.log('Successfully updated CSS files.');
