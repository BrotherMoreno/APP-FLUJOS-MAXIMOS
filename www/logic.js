/* Logica pura (sin DOM): generacion limitada por grado, Edmonds-Karp, parseo de matrices y OCR.
   Funciona en navegador (globales) y en Node (module.exports) para poder probarla. */
(function (root) {
  const rnd = (a, b) => a + Math.floor(Math.random() * (b - a + 1));
  const vacia = n => Array.from({ length: n }, () => new Array(n).fill(0));

  // Red dirigida acíclica 0..n-1 (fuente=0, sumidero=n-1):
  //  - ninguna flecha ENTRA a la fuente ni SALE del sumidero (solo aristas i->j con i<j)
  //  - cada vertice esta en algun camino fuente->sumidero
  //  - grado total (entradas+salidas) <= gradoMax (3 por defecto); solo se supera en 1 si es inevitable
  function generar(n, gradoMax) {
    const G = gradoMax || 3, cap = vacia(n), deg = new Array(n).fill(0), out = new Array(n).fill(0);
    const add = (i, j) => { if (cap[i][j]) return false; cap[i][j] = rnd(3, 25); deg[i]++; deg[j]++; out[i]++; return true; };
    for (let i = 1; i < n; i++) {                                   // 1) arbol que sale de la fuente
      let c = []; for (let j = 0; j < i; j++) if (deg[j] < G) c.push(j);
      if (!c.length) c = [i - 1];
      const sinSalida = c.filter(j => out[j] === 0);
      const pool = (sinSalida.length && Math.random() < 0.7) ? sinSalida : c;
      add(pool[rnd(0, pool.length - 1)], i);
    }
    for (let i = n - 2; i >= 0; i--) if (out[i] === 0) {            // 2) toda hoja (salvo el sumidero) sale hacia adelante
      let c = []; for (let k = i + 1; k < n; k++) if (deg[k] < G) c.push(k);
      add(i, c.length ? c[rnd(0, c.length - 1)] : n - 1);
    }
    let extra = Math.round(n * 0.35);                                // 3) aristas extra (ciclos de ruta) respetando el grado
    for (let t = 0; t < 200 && extra > 0; t++) {
      const i = rnd(0, n - 2), j = rnd(i + 1, n - 1);
      if (deg[i] < G && deg[j] < G && add(i, j)) extra--;
    }
    return cap;
  }

  function bfs(res, n, s, t, padre) {
    padre.fill(-1); padre[s] = s; const q = [s];
    for (let h = 0; h < q.length; h++) { const u = q[h];
      for (let v = 0; v < n; v++) if (padre[v] < 0 && res[u][v] > 0) { padre[v] = u; if (v === t) return true; q.push(v); } }
    return false;
  }
  const copia = m => m.map(f => f.slice());

  function edmondsKarp(cap, n, s, t) {
    const res = copia(cap), fl = vacia(n), pasos = [];
    const base = (o) => Object.assign({ camino: [], cambios: [], cuello: 0, flujo: copia(fl), residual: copia(res), acumulado: 0, flujoMax: 0, iter: 0, esFinal: false, titulo: '' }, o);
    pasos.push(base({ titulo: 'Red inicial (sin flujo)' }));
    let total = 0, it = 0; const padre = new Array(n);
    while (bfs(res, n, s, t, padre)) {
      it++; let cuello = Infinity, camino = [], v = t;
      while (v !== s) { const u = padre[v]; cuello = Math.min(cuello, res[u][v]); camino.push(v); v = u; }
      camino.push(s); camino.reverse();
      const cambios = [];
      for (let k = 0; k < camino.length - 1; k++) {
        const u = camino[k], w = camino[k + 1], rev = cap[u][w] <= 0;
        const c = { desde: u, hasta: w, reversa: rev, resAntes: res[u][w], flAntes: rev ? fl[w][u] : fl[u][w] };
        res[u][w] -= cuello; res[w][u] += cuello;
        if (!rev) fl[u][w] += cuello; else fl[w][u] -= cuello;
        c.resDespues = res[u][w]; c.flDespues = rev ? fl[w][u] : fl[u][w];
        c.nota = rev ? 'reversa (cancela flujo)' : (c.resDespues === 0 ? 'saturada' : 'adelante');
        cambios.push(c);
      }
      total += cuello;
      pasos.push(base({ camino, cambios, cuello, acumulado: total, iter: it, titulo: `Iteración ${it} — camino aumentante (cuello = ${cuello})` }));
    }
    pasos.push(base({ acumulado: total, iter: it, esFinal: true, titulo: 'Red de flujo óptima' }));
    pasos.forEach(p => p.flujoMax = total);
    return { pasos, total };
  }

  // Quita flechas que entran a la fuente o salen del sumidero. Devuelve cuantas quito.
  function sanear(cap, n, s, t) {
    let q = 0;
    for (let i = 0; i < n; i++) { if (cap[i][s] > 0) { cap[i][s] = 0; q++; } if (cap[t][i] > 0) { cap[t][i] = 0; q++; } }
    return q;
  }

  // Texto pegado (Excel/CSV/TSV/espacios) -> matriz de enteros
  function parsearTexto(txt) {
    return txt.replace(/\r/g, '').split('\n').map(l => l.trim()).filter(l => l.length)
      .map(l => l.split(/[\t,;\s]+/).map(x => { const v = parseInt(x.replace(/[^\d-]/g, ''), 10); return isNaN(v) || v < 0 ? 0 : v; }));
  }
  // Lista de aristas: "0 1 10" | "v0->v1:10" | "0,1,10"  (una por linea)
  function parsearAristas(txt) {
    const E = [];
    txt.replace(/\r/g, '').split('\n').forEach(l => { const m = l.match(/\d+/g); if (m && m.length >= 3) E.push([+m[0], +m[1], +m[2]]); });
    return E;
  }
  function aristasAMatriz(E, nMin) {
    let n = Math.max(nMin || 0, ...E.map(e => Math.max(e[0], e[1]) + 1)); const c = vacia(n);
    E.forEach(([a, b, w]) => { if (a !== b) c[a][b] = w; }); return c;
  }

  // ---- OCR: palabras con caja -> matriz ----
  const med = a => { const s = a.slice().sort((x, y) => x - y); return s.length ? s[s.length >> 1] : 0; };
  function matrizDesdePalabras(words, opc) {
    opc = opc || {};
    let W = words.map(w => ({ t: String(w.text).replace(/\D/g, ''), x: (w.bbox.x0 + w.bbox.x1) / 2, y: (w.bbox.y0 + w.bbox.y1) / 2, h: w.bbox.y1 - w.bbox.y0, c: w.confidence || 0 }))
      .filter(w => w.t.length > 0 && w.t.length <= 4);
    if (W.length < 4) return { mat: null, avisos: ['No se reconocieron números suficientes. Acerca la cámara, mejora la luz o usa una foto más nítida.'] };
    const hm = med(W.map(w => w.h)), filas = [];
    W.sort((a, b) => a.y - b.y).forEach(w => { const f = filas[filas.length - 1]; if (f && Math.abs(w.y - f.y) < hm * 0.6) { f.w.push(w); f.y = f.w.reduce((s, q) => s + q.y, 0) / f.w.length; } else filas.push({ y: w.y, w: [w] }); });
    const difs = []; filas.forEach(f => { f.w.sort((a, b) => a.x - b.x); for (let i = 1; i < f.w.length; i++) difs.push(f.w[i].x - f.w[i - 1].x); });
    const pitch = med(difs) || hm * 2, xs = W.map(w => w.x).sort((a, b) => a - b), cols = [];
    xs.forEach(x => { const c = cols[cols.length - 1]; if (c && x - c.x < pitch * 0.5) { c.n++; c.x += (x - c.x) / c.n; } else cols.push({ x, n: 1 }); });
    const R = filas.length, C = cols.length, M = Array.from({ length: R }, () => new Array(C).fill(null));
    filas.forEach((f, i) => f.w.forEach(w => { let j = 0, d = 1e9; cols.forEach((c, k) => { if (Math.abs(c.x - w.x) < d) { d = Math.abs(c.x - w.x); j = k; } });
      if (M[i][j] === null || w.c > M[i][j].c) M[i][j] = { v: parseInt(w.t, 10), c: w.c }; }));
    let g = M.map(r => r.map(x => x ? x.v : 0)), Cc = M.map(r => r.map(x => x ? x.c : 100)); const avisos = [];
    const sec = a => a.every((v, i) => v === a[0] + i);
    const encabezado = opc.encabezados === true || (opc.encabezados !== false && R > 2 && C > 2 &&
      ((sec(g[0].slice(1)) && [0, 1].includes(g[0][1])) || (sec(g.slice(1).map(r => r[0])) && [0, 1].includes(g[1][0]))));
    if (encabezado) { g = g.slice(1).map(r => r.slice(1)); Cc = Cc.slice(1).map(r => r.slice(1)); avisos.push('Se ignoraron la fila y la columna de encabezados.'); }
    const n = Math.max(g.length, g[0].length), mat = vacia(n);
    for (let i = 0; i < g.length; i++) for (let j = 0; j < g[0].length; j++) mat[i][j] = g[i][j];
    for (let i = 0; i < n; i++) mat[i][i] = 0;
    if (g.length !== g[0].length) avisos.push(`La tabla detectada es de ${g.length}×${g[0].length}; se completó con ceros hasta ${n}×${n}. Revísala.`);
    if (opc.n && opc.n !== n) avisos.push(`Detecté ${n} vértices pero tenías n = ${opc.n}. Revisa antes de aplicar.`);
    const bajos = Cc.flat().filter(c => c < 45).length; if (bajos) avisos.push(`${bajos} número(s) con baja confianza: corrígelos en la vista previa.`);
    avisos.push('Las celdas sin número se toman como 0 (sin arista). Verifica que no falte ninguna capacidad.');
    return { mat, avisos };
  }

  const API = { generar, edmondsKarp, sanear, parsearTexto, parsearAristas, aristasAMatriz, matrizDesdePalabras, vacia, copia };
  if (typeof module !== 'undefined' && module.exports) module.exports = API; else root.FM = API;
})(typeof window !== 'undefined' ? window : globalThis);
