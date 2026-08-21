import { useEffect, useState } from 'react';
import { listJobs, deleteJob } from '../api';
import { Job } from '../types';
import './History.css';

interface HistoryProps {
  onViewJob: (jobId: string) => void;
}

export default function History({ onViewJob }: HistoryProps) {
  const [jobs, setJobs] = useState<Job[]>([]);
  const [loading, setLoading] = useState(true);

  const fetchJobs = async () => {
    try {
      const data = await listJobs();
      setJobs(data);
    } catch (err) {
      console.error(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchJobs();
  }, []);

  const handleDelete = async (e: React.MouseEvent, jobId: string) => {
    e.stopPropagation();
    if (confirm('Delete this job?')) {
      await deleteJob(jobId);
      fetchJobs();
    }
  };

  if (loading) return <div className="history-page">Loading...</div>;

  return (
    <div className="history-page">
      <h2>Job History</h2>
      {jobs.length === 0 ? (
        <p>No past jobs found.</p>
      ) : (
        <div className="jobs-list card">
          <table>
            <thead>
              <tr>
                <th>Job ID</th>
                <th>Date</th>
                <th>Files</th>
                <th>Status</th>
                <th>Actions</th>
              </tr>
            </thead>
            <tbody>
              {jobs.map(job => (
                <tr key={job.id} onClick={() => onViewJob(job.id)}>
                  <td className="job-id">{job.id.substring(0,8)}...</td>
                  <td>{new Date(job.createdAt).toLocaleString()}</td>
                  <td>{job.fileCount}</td>
                  <td><span className={`status ${job.status}`}>{job.status}</span></td>
                  <td>
                    <button className="delete-btn" onClick={(e) => handleDelete(e, job.id)}>Delete</button>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </div>
  );
}
