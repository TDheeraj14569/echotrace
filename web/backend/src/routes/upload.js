const express = require('express');
const multer = require('multer');
const path = require('path');
const { v4: uuidv4 } = require('uuid');
const db = require('../db');

const router = express.Router();

const UPLOAD_DIR = path.join(__dirname, '..', '..', 'uploads');

// Configure multer for file uploads with size limits
const storage = multer.diskStorage({
    destination: (req, file, cb) => {
        const jobId = req.jobId || uuidv4();
        req.jobId = jobId;
        const jobDir = path.join(UPLOAD_DIR, jobId);
        const fs = require('fs');
        if (!fs.existsSync(jobDir)) fs.mkdirSync(jobDir, { recursive: true });
        cb(null, jobDir);
    },
    filename: (req, file, cb) => {
        // Sanitize filename - prevent path traversal
        const safe = file.originalname.replace(/[^a-zA-Z0-9._-]/g, '_');
        cb(null, safe);
    }
});

const upload = multer({
    storage,
    limits: {
        fileSize: 1024 * 1024, // 1MB per file
        files: 100             // max 100 files per upload
    },
    fileFilter: (req, file, cb) => {
        const ext = path.extname(file.originalname).toLowerCase();
        const allowed = ['.cpp', '.cc', '.cxx', '.h', '.hpp', '.c',
                         '.py', '.pyw', '.java', '.js', '.jsx', '.ts', '.tsx'];
        if (allowed.includes(ext)) {
            cb(null, true);
        } else {
            cb(new Error(`Unsupported file type: ${ext}`));
        }
    }
});

// Detect language from file extension
function detectLanguage(filename) {
    const ext = path.extname(filename).toLowerCase();
    const map = {
        '.cpp': 'cpp', '.cc': 'cpp', '.cxx': 'cpp', '.h': 'cpp', '.hpp': 'cpp', '.c': 'cpp',
        '.py': 'python', '.pyw': 'python',
        '.java': 'java',
        '.js': 'javascript', '.jsx': 'javascript', '.ts': 'javascript', '.tsx': 'javascript'
    };
    return map[ext] || 'unknown';
}

// POST /api/upload - Upload source files for analysis
router.post('/', upload.array('files', 100), (req, res) => {
    if (!req.files || req.files.length < 2) {
        return res.status(400).json({
            error: 'At least 2 source files are required for comparison'
        });
    }

    const jobId = req.jobId;
    const config = {
        k: parseInt(req.body.k) || 5,
        window: parseInt(req.body.window) || 4,
        threshold: parseFloat(req.body.threshold) || 0,
        threads: parseInt(req.body.threads) || 0,
        format: req.body.format || 'json'
    };

    const files = req.files.map(f => ({
        originalName: f.originalname,
        storedPath: f.path,
        language: detectLanguage(f.originalname),
        size: f.size
    }));

    try {
        db.createJob(jobId, config, files);
        res.status(201).json({
            id: jobId,
            status: 'pending',
            fileCount: files.length,
            config
        });
    } catch (err) {
        res.status(500).json({ error: 'Failed to create job: ' + err.message });
    }
});

// Error handler for multer
router.use((err, req, res, next) => {
    if (err instanceof multer.MulterError) {
        if (err.code === 'LIMIT_FILE_SIZE') {
            return res.status(413).json({ error: 'File too large (max 1MB)' });
        }
        if (err.code === 'LIMIT_FILE_COUNT') {
            return res.status(413).json({ error: 'Too many files (max 100)' });
        }
        return res.status(400).json({ error: err.message });
    }
    if (err) {
        return res.status(400).json({ error: err.message });
    }
    next();
});

module.exports = router;
