// Ventana de escritorio. Sirve www/ por http://127.0.0.1 (necesario para cámara y OCR sin conexión).
const { app, BrowserWindow, session, Menu } = require('electron');
const http = require('http'), fs = require('fs'), path = require('path');
const MIME = { '.html': 'text/html; charset=utf-8', '.js': 'text/javascript', '.gz': 'application/gzip', '.json': 'application/json', '.wasm': 'application/wasm' };
function servir() {
  return new Promise(ok => {
    const raiz = path.join(__dirname, 'www');
    const s = http.createServer((q, r) => {
      let f = path.normalize(path.join(raiz, decodeURIComponent(q.url.split('?')[0]) === '/' ? 'index.html' : decodeURIComponent(q.url.split('?')[0])));
      if (!f.startsWith(raiz)) { r.writeHead(403); return r.end(); }
      fs.readFile(f, (e, d) => { if (e) { r.writeHead(404); return r.end(); } r.writeHead(200, { 'Content-Type': MIME[path.extname(f)] || 'application/octet-stream' }); r.end(d); });
    }).listen(0, '127.0.0.1', () => ok(s.address().port));
  });
}
app.whenReady().then(async () => {
  const puerto = await servir();
  session.defaultSession.setPermissionRequestHandler((wc, perm, cb) => cb(perm === 'media'));
  Menu.setApplicationMenu(null);
  const w = new BrowserWindow({ width: 1366, height: 820, backgroundColor: '#1e1e2e', title: 'Flujo Máximo — Edmonds-Karp', autoHideMenuBar: true });
  w.maximize(); w.loadURL('http://127.0.0.1:' + puerto + '/index.html');
});
app.on('window-all-closed', () => app.quit());
