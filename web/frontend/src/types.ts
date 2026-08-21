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
  file1: string;
  file2: string;
  similarity: number;
  fragments?: any[];
}

export interface AnalysisResult {
  jobId: string;
  status: string;
  stats: {
    filesAnalyzed: number;
    comparisonsMade: number;
    highestSimilarity: number;
    avgSimilarity: number;
    durationMs: number;
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
