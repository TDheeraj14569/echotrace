const test = require('node:test');
const assert = require('node:assert');
const request = require('supertest');

const app = require('../server');
const db = require('../db');

test('GET /api/health should return ok', async (t) => {
    const res = await request(app).get('/api/health');
    assert.strictEqual(res.statusCode, 200);
    assert.strictEqual(res.body.status, 'ok');
});

test('GET /api/results/invalid should return 404', async (t) => {
    const res = await request(app).get('/api/results/invalid');
    assert.strictEqual(res.statusCode, 404);
});

test('GET /api/analysis/invalid/status should return 404', async (t) => {
    const res = await request(app).get('/api/analysis/invalid/status');
    assert.strictEqual(res.statusCode, 404);
});

test('Database interface loads', (t) => {
    assert.ok(typeof db.getJob === 'function');
    assert.ok(typeof db.createJob === 'function');
});
