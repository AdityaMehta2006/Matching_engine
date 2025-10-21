const WebSocket = require('ws');
const http = require('http');
const POLL_URL = process.env.POLL_URL || 'http://localhost:8080/events/poll';
const POLL_INTERVAL_MS = process.env.POLL_INTERVAL_MS ? parseInt(process.env.POLL_INTERVAL_MS) : 200;
const PORT = process.env.PORT ? parseInt(process.env.PORT) : 8081;

let wss = null;
try{
  wss = new WebSocket.Server({ port: PORT });
  console.log('WebSocket bridge listening on ws://localhost:' + PORT);
  wss.on('connection', (ws) => {
    console.log('bridge: client connected');
    ws.on('close', () => console.log('bridge: client disconnected'));
  });
  wss.on('error', (err) => {
    console.error('bridge: websocket server error', err && err.code ? err.code : err);
    // if address in use, exit gracefully so orchestrator (demo) can retry or report
    process.exit(1);
  });
}catch(err){
  console.error('bridge: failed to start websocket server:', err && err.code ? err.code : err);
  process.exit(1);
}

async function pollOnce(){
  return new Promise((resolve) => {
    const req = http.get(POLL_URL, (res) => {
      if(res.statusCode !== 200){ res.resume(); return resolve(null); }
      let body = '';
      res.setEncoding('utf8');
      res.on('data', (d) => body += d);
      res.on('end', () => resolve(body));
    });
    req.on('error', () => resolve(null));
    req.setTimeout(5000, () => { req.abort(); resolve(null); });
  });
}

let running = true;
(async function loop(){
  while(running){
    try{
      const body = await pollOnce();
      if(body){
        wss.clients.forEach((c) => { if(c.readyState === WebSocket.OPEN) c.send(body); });
      }
    }catch(e){ /* swallow */ }
    await new Promise(r => setTimeout(r, POLL_INTERVAL_MS));
  }
})();

process.on('SIGINT', ()=>{ running=false; wss.close(() => process.exit(0)); });
process.on('SIGTERM', ()=>{ running=false; wss.close(() => process.exit(0)); });
