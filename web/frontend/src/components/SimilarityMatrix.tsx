import { useMemo } from 'react';
import './SimilarityMatrix.css';

interface SimilarityMatrixProps {
  matches: any[];
}

export default function SimilarityMatrix({ matches }: SimilarityMatrixProps) {
  const { files, matrix } = useMemo(() => {
    if (!matches) return { files: [], matrix: {} };
    const files = Array.from(new Set(matches.flatMap(m => [m.file_a, m.file_b]))).sort();
    
    const matrix: Record<string, Record<string, number>> = {};
    files.forEach(f => {
      matrix[f] = {};
      matrix[f][f] = 100; // 100% with itself
    });

    matches.forEach(m => {
      matrix[m.file_a][m.file_b] = m.similarity;
      matrix[m.file_b][m.file_a] = m.similarity;
    });

    return { files, matrix };
  }, [matches]);

  if (files.length === 0) return <div>No matrix data</div>;

  const getColor = (sim: number) => {
    // 0 = #161b22, 100 = #da3633 (red)
    const ratio = sim / 100;
    return `rgba(218, 54, 51, ${ratio})`;
  };

  return (
    <div className="matrix-wrapper">
      <table className="sim-matrix">
        <thead>
          <tr>
            <th></th>
            {files.map(f => (
              <th key={f} title={f}><div className="rotated-th"><span>{f}</span></div></th>
            ))}
          </tr>
        </thead>
        <tbody>
          {files.map(row => (
            <tr key={row}>
              <td className="row-header" title={row}>{row}</td>
              {files.map(col => {
                const sim = matrix[row][col] || 0;
                return (
                  <td 
                    key={col} 
                    style={{ backgroundColor: getColor(sim) }}
                    title={`${row} vs ${col}\nSimilarity: ${sim.toFixed(1)}%`}
                  >
                    {sim.toFixed(0)}%
                  </td>
                );
              })}
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}
