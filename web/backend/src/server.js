const express = require('express');
const cors = require('cors');
const path = require('path');
const fs = require('fs');

const uploadRoutes = require('./routes/upload');
const analysisRoutes = require('./routes/analysis');
const resultsRoutes = require('./routes/results');
const db = require('./db');

const app = express();
const PORT = process.env.PORT || 3001;

// Ensure upload directory exists
const UPLOAD_DIR = path.join(__dirname, '..', 'uploads');
if (!fs.existsSync(UPLOAD_DIR)) {
    fs.mkdirSync(UPLOAD_DIR, { recursive: true });
}

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// API Routes
app.use('/api/upload', uploadRoutes);
app.use('/api/analysis', analysisRoutes);
app.use('/api/results', resultsRoutes);

// Health check
app.get('/api/health', (req, res) => {
    res.json({
        status: 'ok',
        version: '1.0.0',
        engine: 'echotrace',
        timestamp: new Date().toISOString()
    });
});

// Serve static frontend in production
const frontendBuild = path.join(__dirname, '..', '..', 'frontend', 'dist');
if (fs.existsSync(frontendBuild)) {
    app.use(express.static(frontendBuild));
    app.get('*', (req, res) => {
        if (!req.path.startsWith('/api')) {
            res.sendFile(path.join(frontendBuild, 'index.html'));
        }
    });
}

// Initialize database and start server
db.initialize();

app.listen(PORT, () => {
    console.log(`EchoTrace backend running on http://localhost:${PORT}`);
});

module.exports = app;
