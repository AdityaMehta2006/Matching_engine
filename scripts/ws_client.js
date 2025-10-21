const WebSocket = require('ws');
const URL = process.env.URL || 'ws://localhost:8081';

const ws = new WebSocket(URL);
ws.on('open', () => console.log('ws client connected to', URL));
ws.on('message', (data) => console.log('ws client received:', data.toString()));
ws.on('close', () => console.log('ws client closed'));
ws.on('error', (e) => console.error('ws client error', e));
