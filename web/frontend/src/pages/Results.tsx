import { useEffect, useState } from 'react';
import { getResults, getJobStatus } from '../api';
import { AnalysisResult, FileMatch } from '../types';
import SimilarityMatrix from '../components/SimilarityMatrix';
import SourceCompare from '../components/SourceCompare';
import './Results.css';

export default function Results({ jobId }: { jobId: string | null }) {
  const [result, setResult] = useState<AnalysisResult | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [selectedMatch, setSelectedMatch] = useState<FileMatch | null>(null);
  const [status, setStatus] = useState<string>('pending');

  useEffect(() => {
    if (!jobId) return;

    const pollInterval = setInterval(async () => {
      try {
        const statusRes = await getJobStatus(jobId);
        setStatus(statusRes.status);
        if (statusRes.status === 'completed') {
          clearInterval(pollInterval);
          const res = await getResults(jobId);
          setResult(res);
          setLoading(false);
        } else if (statusRes.status === 'failed') {
          clearInterval(pollInterval);
          setError('Analysis failed');
          setLoading(false);
        }
      } catch (err: any) {
        clearInterval(pollInterval);
        setError(err.message || 'Error checking status');
        setLoading(false);
      }
    }, 2000);

    return () => clearInterval(pollInterval);
  }, [jobId]);

  if (!jobId) return <div className="results-page">No job selected</div>;
  if (loading) return <div className="results-page">Loading analysis (Status: {status})...</div>;
  if (error) return <div className="results-page error">{error}</div>;
  if (!result) return null;

  return (
    <div className="results-page">
      <h2>Analysis Results</h2>
      
      <div className="stats-grid">
        <div className="stat-card">
          <span className="stat-label">Files Analyzed</span>
          <span className="stat-value">{result.summary?.files || result.fileCount}</span>
        </div>
        <div className="stat-card">
          <span className="stat-label">Comparisons</span>
          <span className="stat-value">{result.summary?.comparisons || 0}</span>
        </div>
        <div className="stat-card">
          <span className="stat-label">Highest Similarity</span>
          <span className="stat-value">{(result.summary?.highest_similarity || 0).toFixed(1)}%</span>
        </div>
        <div className="stat-card">
          <span className="stat-label">Avg Similarity</span>
          <span className="stat-value">{(result.summary?.average_similarity || 0).toFixed(1)}%</span>
        </div>
        <div className="stat-card">
          <span className="stat-label">Duration</span>
          <span className="stat-value">{((result.duration_ms || result.durationMs || 0) / 1000).toFixed(2)}s</span>
        </div>
      </div>

      <div className="results-content">
        <div className="matches-section card">
          <h3>Top Matches</h3>
          <div className="matches-list">
            <table>
              <thead>
                <tr>
                  <th>File 1</th>
                  <th>File 2</th>
                  <th>Similarity</th>
                </tr>
              </thead>
              <tbody>
                {(result.matches || []).sort((a: any,b: any) => b.similarity - a.similarity).map((m: any, i: number) => (
                  <tr key={i} onClick={() => setSelectedMatch(m)} className={selectedMatch === m ? 'selected' : ''}>
                    <td>{m.file_a}</td>
                    <td>{m.file_b}</td>
                    <td>
                      <span className="sim-badge" style={{backgroundColor: `rgba(218, 54, 51, ${m.similarity / 100})`}}>
                        {(m.similarity).toFixed(1)}%
                      </span>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>
        
        <div className="matrix-section card">
          <h3>Similarity Matrix</h3>
          <SimilarityMatrix matches={result.matches} />
        </div>
      </div>

      {selectedMatch && (
        <div className="compare-section card">
          <h3>Source Comparison</h3>
          <SourceCompare match={selectedMatch} jobId={jobId} />
        </div>
      )}
    </div>
  );
}
