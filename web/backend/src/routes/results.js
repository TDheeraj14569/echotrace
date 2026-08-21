const express = require('express');
const db = require('../db');

const router = express.Router();

// GET /api/results/:jobId - Get analysis results
router.get('/:jobId', (req, res) => {
    const job = db.getJob(req.params.jobId);
    if (!job) {
        return res.status(404).json({ error: 'Job not found' });
    }

    if (job.status !== 'completed') {
        return res.json({
            id: job.id,
            status: job.status,
            error: job.error || undefined
        });
    }

    const result = (typeof job.result === 'string') ? JSON.parse(job.result) : (job.result || {});
    const files = db.getJobFiles(job.id);

    res.json({
        id: job.id,
        status: 'completed',
        durationMs: job.duration_ms,
        fileCount: job.file_count,
        files: files.map(f => ({
            name: f.original_name,
            language: f.language,
            size: f.size_bytes
        })),
        ...result
    });
});

// GET /api/results - List all jobs
router.get('/', (req, res) => {
    const limit = Math.min(parseInt(req.query.limit) || 50, 200);
    const jobs = db.listJobs(limit);

    res.json(jobs.map(j => ({
        id: j.id,
        status: j.status,
        fileCount: j.file_count,
        createdAt: j.created_at,
        durationMs: j.duration_ms,
        error: j.error || undefined
    })));
});

// DELETE /api/results/:jobId - Delete a job and its files
router.delete('/:jobId', (req, res) => {
    const job = db.getJob(req.params.jobId);
    if (!job) {
        return res.status(404).json({ error: 'Job not found' });
    }

    const path = require('path');
    const fs = require('fs');
    const uploadDir = path.join(__dirname, '..', '..', 'uploads', job.id);
    if (fs.existsSync(uploadDir)) {
        fs.rmSync(uploadDir, { recursive: true, force: true });
    }

    db.deleteJob(job.id);
    res.json({ message: 'Job deleted', id: job.id });
});

// GET /api/results/:jobId/file/:filename - Fetch raw file content
router.get('/:jobId/file/:filename', (req, res) => {
    const { jobId, filename } = req.params;
    if (filename.includes('..') || filename.includes('/') || filename.includes('\\')) {
        return res.status(400).json({ error: 'Invalid filename' });
    }
    const path = require('path');
    const fs = require('fs');
    const filePath = path.join(__dirname, '..', '..', 'uploads', jobId, filename);
    
    if (!fs.existsSync(filePath)) {
        return res.status(404).json({ error: 'File not found' });
    }
    res.sendFile(filePath);
});

module.exports = router;

