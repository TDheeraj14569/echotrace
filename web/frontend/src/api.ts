import { AnalysisConfig, JobResponse, JobStatus, AnalysisResult, Job, HealthResponse } from './types';

const API_BASE = '/api';

export async function uploadFiles(files: File[], config: AnalysisConfig): Promise<JobResponse> {
  const formData = new FormData();
  files.forEach(file => formData.append('files', file));
  if (config.k) formData.append('k', config.k.toString());
  if (config.window) formData.append('window', config.window.toString());
  // The slider stores threshold as 0.0–1.0; the CLI expects 0–100 (percent).
  if (config.threshold !== undefined) {
    const pct = Math.round(config.threshold * 100);
    formData.append('threshold', pct.toString());
  }
  if (config.threads) formData.append('threads', config.threads.toString());
  if (config.format) formData.append('format', config.format);

  const res = await fetch(`${API_BASE}/upload`, {
    method: 'POST',
    body: formData,
  });
  if (!res.ok) throw new Error('Upload failed');
  return res.json();
}

export async function startAnalysis(jobId: string): Promise<void> {
  const res = await fetch(`${API_BASE}/analysis/${jobId}/start`, { method: 'POST' });
  if (!res.ok) throw new Error('Failed to start analysis');
}

export async function getJobStatus(jobId: string): Promise<JobStatus> {
  const res = await fetch(`${API_BASE}/analysis/${jobId}/status`);
  if (!res.ok) throw new Error('Failed to get status');
  return res.json();
}

export async function getResults(jobId: string): Promise<AnalysisResult> {
  const res = await fetch(`${API_BASE}/results/${jobId}`);
  if (!res.ok) throw new Error('Failed to get results');
  return res.json();
}

export async function getFileContent(jobId: string, filename: string): Promise<string> {
  const res = await fetch(`${API_BASE}/results/${jobId}/file/${encodeURIComponent(filename)}`);
  if (!res.ok) throw new Error('Failed to get file content');
  return res.text();
}

export async function listJobs(): Promise<Job[]> {
  const res = await fetch(`${API_BASE}/results`);
  if (!res.ok) throw new Error('Failed to list jobs');
  return res.json();
}

export async function deleteJob(jobId: string): Promise<void> {
  const res = await fetch(`${API_BASE}/results/${jobId}`, { method: 'DELETE' });
  if (!res.ok) throw new Error('Failed to delete job');
}

export async function healthCheck(): Promise<HealthResponse> {
  const res = await fetch(`${API_BASE}/health`);
  if (!res.ok) throw new Error('Health check failed');
  return res.json();
}
