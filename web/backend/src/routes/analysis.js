const express = require('express');
const { execFile } = require('child_process');
const path = require('path');
const fs = require('fs');
const db = require('../db');

const router = express.Router();

// Resolve echotrace CLI binary
const ENGINE_PATH = path.resolve(__dirname, '..', '..', '..', '..', 'build', 'bin', 'echotrace.exe');

// POST /api/analysis/:jobId/start - Start analysis for a job
router.post('/:jobId/start', async (req, res) => {
    const { jobId } = req.params;
    const job = db.getJob(jobId);

    if (!job) {
        return res.status(404).json({ error: 'Job not found' });
    }
    if (job.status === 'running') {
        return res.status(409).json({ error: 'Analysis already running' });
    }
    if (job.status === 'completed') {
        return res.status(409).json({ error: 'Analysis already completed' });
    }

    // Verify engine binary exists
    if (!fs.existsSync(ENGINE_PATH)) {
        return res.status(503).json({ error: 'EchoTrace engine not found at: ' + ENGINE_PATH });
    }

    let config;
    try {
        config = typeof job.config === 'string' ? JSON.parse(job.config) : job.config;
    } catch (err) {
        return res.status(500).json({ error: 'Invalid job configuration' });
    }

    const uploadDir = path.join(__dirname, '..', '..', 'uploads', jobId);

    db.updateJobStatus(jobId, 'running');

    // Run echotrace CLI as subprocess
    const args = [
        uploadDir,
        '--format', 'json',
        '--k', String(config.k || 5),
        '-w', String(config.window || 4)
    ];
    if (config.threads) {
        args.push('--threads', String(config.threads));
    }
    if (config.threshold > 0) {
        args.push('--threshold', String(config.threshold));
    }

    const startTime = Date.now();

    execFile(ENGINE_PATH, args, {
        maxBuffer: 50 * 1024 * 1024, // 50MB output buffer
        timeout: 300000               // 5 minute timeout
    }, (error, stdout, stderr) => {
        const duration = Date.now() - startTime;

        if (error) {
            db.updateJobStatus(jobId, 'failed', {
                error: error.message + (stderr ? '\n' + stderr : ''),
                duration_ms: duration
            });
            return;
        }

        try {
            const result = JSON.parse(stdout);
            db.updateJobStatus(jobId, 'completed', {
                result: result,
                duration_ms: duration
            });
        } catch (parseErr) {
            db.updateJobStatus(jobId, 'failed', {
                error: 'Failed to parse engine output: ' + parseErr.message,
                duration_ms: duration
            });
        }
    });

    res.json({
        id: jobId,
        status: 'running',
        message: 'Analysis started'
    });
});

// GET /api/analysis/:jobId/status - Check analysis status
router.get('/:jobId/status', (req, res) => {
    const job = db.getJob(req.params.jobId);
    if (!job) {
        return res.status(404).json({ error: 'Job not found' });
    }

    const response = {
        id: job.id,
        status: job.status,
        fileCount: job.file_count,
        createdAt: job.created_at,
        updatedAt: job.updated_at
    };

    if (job.duration_ms) response.durationMs = job.duration_ms;
    if (job.error) response.error = job.error;

    res.json(response);
});

module.exports = router;
