const fs = require('fs');
const path = require('path');

const DB_PATH = path.join(__dirname, '..', 'data', 'jobs.json');

function loadDb() {
    try {
        if (fs.existsSync(DB_PATH)) {
            return JSON.parse(fs.readFileSync(DB_PATH, 'utf8'));
        }
    } catch { /* corrupted, start fresh */ }
    return { jobs: {} };
}

function saveDb(data) {
    const dir = path.dirname(DB_PATH);
    if (!fs.existsSync(dir)) fs.mkdirSync(dir, { recursive: true });
    fs.writeFileSync(DB_PATH, JSON.stringify(data, null, 2));
}

function initialize() {
    const data = loadDb();
    saveDb(data);
}

function createJob(id, config, files) {
    const data = loadDb();
    data.jobs[id] = {
        id,
        status: 'pending',
        created_at: new Date().toISOString(),
        updated_at: new Date().toISOString(),
        config,
        file_count: files.length,
        files: files.map(f => ({
            original_name: f.originalName,
            stored_path: f.storedPath,
            language: f.language || null,
            size_bytes: f.size || 0
        })),
        error: null,
        result: null,
        duration_ms: null
    };
    saveDb(data);
}

function updateJobStatus(id, status, extra = {}) {
    const data = loadDb();
    const job = data.jobs[id];
    if (!job) return;

    job.status = status;
    job.updated_at = new Date().toISOString();

    if (extra.error !== undefined) job.error = extra.error;
    if (extra.result !== undefined) job.result = extra.result;
    if (extra.duration_ms !== undefined) job.duration_ms = extra.duration_ms;

    saveDb(data);
}

function getJob(id) {
    const data = loadDb();
    return data.jobs[id] || null;
}

function getJobFiles(jobId) {
    const job = getJob(jobId);
    return job ? (job.files || []) : [];
}

function listJobs(limit = 50) {
    const data = loadDb();
    return Object.values(data.jobs)
        .sort((a, b) => new Date(b.created_at) - new Date(a.created_at))
        .slice(0, limit);
}

function deleteJob(id) {
    const data = loadDb();
    delete data.jobs[id];
    saveDb(data);
}

module.exports = {
    initialize,
    createJob,
    updateJobStatus,
    getJob,
    getJobFiles,
    listJobs,
    deleteJob
};
