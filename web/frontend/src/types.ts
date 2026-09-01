export interface AnalysisConfig {
  k?: number;
  window?: number;
  threshold?: number;
  threads?: number;
  format?: string;
}

export interface JobResponse {
  id: string;
  status: string;
  fileCount: number;
  config?: AnalysisConfig;
}

export interface JobStatus {
  status: 'pending' | 'running' | 'completed' | 'failed';
  error?: string;
}

export interface FileMatch {
  file_a: string;
  file_b: string;
  similarity: number;
  fragments?: any[];
}

export interface AnalysisResult {
  jobId: string;
  status: string;
  fileCount?: number;
  durationMs?: number;
  duration_ms?: number;
  summary?: {
    files?: number;
    comparisons?: number;
    highest_similarity?: number;
    average_similarity?: number;
  };
  matches: FileMatch[];
}

export interface Job {
  id: string;
  status: string;
  createdAt: string;
  fileCount: number;
}

export interface HealthResponse {
  status: string;
}
