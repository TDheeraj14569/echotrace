import './Header.css'

interface HeaderProps {
  current: string;
  onNavigate: (page: 'upload' | 'results' | 'history') => void;
}

export default function Header({ current, onNavigate }: HeaderProps) {
  return (
    <header className="app-header">
      <div className="header-brand">
        <h1>EchoTrace</h1>
      </div>
      <nav className="header-nav">
        <button 
          className={current === 'upload' ? 'active' : ''} 
          onClick={() => onNavigate('upload')}
        >
          New Analysis
        </button>
        <button 
          className={current === 'history' ? 'active' : ''} 
          onClick={() => onNavigate('history')}
        >
          History
        </button>
      </nav>
    </header>
  )
}
