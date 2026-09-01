import { useState, useRef } from 'react';
import { uploadFiles, startAnalysis } from '../api';
import './Upload.css';

interface UploadProps {
  onAnalysisStarted: (jobId: string) => void;
}

export default function Upload({ onAnalysisStarted }: UploadProps) {
  const [files, setFiles] = useState<File[]>([]);
  const [config, setConfig] = useState({ k: 15, window: 10, threshold: 0.8 });
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const fileInputRef = useRef<HTMLInputElement>(null);

  const handleDrop = (e: React.DragEvent) => {
    e.preventDefault();
    const droppedFiles = Array.from(e.dataTransfer.files);
    setFiles(prev => [...prev, ...droppedFiles]);
  };

  const handleFileSelect = (e: React.ChangeEvent<HTMLInputElement>) => {
    if (e.target.files) {
      setFiles(prev => [...prev, ...Array.from(e.target.files!)]);
    }
  };

  const handleSubmit = async () => {
    if (files.length < 2) {
      setError("Please select at least 2 files to analyze.");
      return;
    }
    setLoading(true);
    setError(null);
    try {
      const res = await uploadFiles(files, config);
      const jobId = res.id;
      await startAnalysis(jobId);
      onAnalysisStarted(jobId);
    } catch (err: any) {
      setError(err.message || 'An error occurred');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="upload-page">
      <h2>New Analysis</h2>
      
      <div 
        className="drop-zone"
        onDragOver={(e) => e.preventDefault()}
        onDrop={handleDrop}
        onClick={() => fileInputRef.current?.click()}
      >
        <p>Drag and drop files here, or click to select files</p>
        <p className="supported-exts">Supported: .cpp, .py, .java, .js, .ts</p>
        <input 
          type="file" 
          multiple 
          hidden 
          ref={fileInputRef} 
          onChange={handleFileSelect} 
        />
      </div>

      {files.length > 0 && (
        <div className="file-list card">
          <h3>Selected Files ({files.length})</h3>
          <ul>
            {files.map((f, i) => (
              <li key={i}>{f.name} <span className="file-size">{(f.size / 1024).toFixed(1)} KB</span></li>
            ))}
          </ul>
        </div>
      )}

      <div className="config-panel card">
        <h3>Configuration</h3>
        <div className="config-group">
          <label>K-gram Size: {config.k}</label>
          <input 
            type="range" min="5" max="50" value={config.k}
            onChange={(e) => setConfig({...config, k: parseInt(e.target.value)})}
          />
        </div>
        <div className="config-group">
          <label>Window Size: {config.window}</label>
          <input 
            type="range" min="5" max="50" value={config.window}
            onChange={(e) => setConfig({...config, window: parseInt(e.target.value)})}
          />
        </div>
        <div className="config-group">
          <label>Similarity Threshold: {(config.threshold * 100).toFixed(0)}%</label>
          <input 
            type="range" min="0" max="1" step="0.01" value={config.threshold}
            onChange={(e) => setConfig({...config, threshold: parseFloat(e.target.value)})}
          />
        </div>
      </div>

      {error && <div className="error-msg">{error}</div>}

      <div className="actions">
        <button className="primary analyze-btn" onClick={handleSubmit} disabled={loading || files.length < 2}>
          {loading ? 'Processing...' : 'Upload & Analyze'}
        </button>
      </div>
    </div>
  );
}
