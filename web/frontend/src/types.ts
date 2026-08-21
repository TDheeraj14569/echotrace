export interface AnalysisConfig {
  k?: number;
  window?: number;
  threshold?: number;
  threads?: number;
  format?: string;
}

export interface JobResponse {
  jobId: string;
  message: string;
}

export interface JobStatus {
  status: 'pending' | 'processing' | 'completed' | 'failed';
  progress: number;
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
