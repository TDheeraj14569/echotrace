import { useState } from 'react'
import Header from './components/Header'
import Upload from './pages/Upload'
import Results from './pages/Results'
import History from './pages/History'
import './App.css'

type Page = 'upload' | 'results' | 'history';

function App() {
  const [currentPage, setCurrentPage] = useState<Page>('upload');
  const [currentJobId, setCurrentJobId] = useState<string | null>(null);

  const navigateTo = (page: Page, jobId?: string) => {
    setCurrentPage(page);
    if (jobId) {
      setCurrentJobId(jobId);
    }
  };

  return (
    <div className="app-container">
      <Header current={currentPage} onNavigate={(p) => navigateTo(p)} />
      
      <main className="main-content">
        {currentPage === 'upload' && <Upload onAnalysisStarted={(jobId) => navigateTo('results', jobId)} />}
        {currentPage === 'results' && <Results jobId={currentJobId} />}
        {currentPage === 'history' && <History onViewJob={(jobId) => navigateTo('results', jobId)} />}
      </main>
    </div>
  )
}

export default App
