import { useEffect, useState } from 'react';
import { FileMatch } from '../types';
import { getFileContent } from '../api';
import './SourceCompare.css';

interface SourceCompareProps {
  match: FileMatch;
  jobId: string;
}

export default function SourceCompare({ match, jobId }: SourceCompareProps) {
  const [code1, setCode1] = useState<string>('Loading...');
  const [code2, setCode2] = useState<string>('Loading...');

  useEffect(() => {
    let mounted = true;
    
    async function fetchCodes() {
      try {
        setCode1('Loading...');
        setCode2('Loading...');
        const [text1, text2] = await Promise.all([
          getFileContent(jobId, match.file_a),
          getFileContent(jobId, match.file_b)
        ]);
        if (mounted) {
          setCode1(text1);
          setCode2(text2);
        }
      } catch (err) {
        if (mounted) {
          setCode1('Failed to load source code.');
          setCode2('Failed to load source code.');
        }
      }
    }

    if (match && jobId) {
      fetchCodes();
    }

    return () => { mounted = false; };
  }, [match, jobId]);

  const renderHighlighted = (text: string, isFileA: boolean) => {
    if (!match.fragments || match.fragments.length === 0 || text.startsWith('Loading') || text.startsWith('Failed')) {
      return text;
    }

    const highlighted = new Array(text.length).fill(false);
    match.fragments.forEach(f => {
      const offset = isFileA ? f.a_offset : f.b_offset;
      const length = isFileA ? f.a_length : f.b_length;
      if (typeof offset === 'number' && typeof length === 'number') {
        for (let i = offset; i < offset + length && i < text.length; i++) {
          highlighted[i] = true;
        }
      }
    });

    const elements: React.ReactNode[] = [];
    if (text.length === 0) return elements;

    let currentIsHighlighted = highlighted[0];
    let currentStart = 0;

    for (let i = 1; i <= text.length; i++) {
      if (i === text.length || highlighted[i] !== currentIsHighlighted) {
        const substring = text.substring(currentStart, i);
        if (currentIsHighlighted) {
          elements.push(<mark key={currentStart} className="match-highlight">{substring}</mark>);
        } else {
          elements.push(<span key={currentStart}>{substring}</span>);
        }
        if (i < text.length) {
          currentIsHighlighted = highlighted[i];
          currentStart = i;
        }
      }
    }
    return elements;
  };

  return (
    <div className="source-compare">
      <div className="source-pane">
        <h4>{match.file_a}</h4>
        <pre><code>{renderHighlighted(code1, true)}</code></pre>
      </div>
      <div className="source-pane">
        <h4>{match.file_b}</h4>
        <pre><code>{renderHighlighted(code2, false)}</code></pre>
      </div>
    </div>
  );
}
