// Pacman Maze: Survival in the Dark - Web Edition
(() => {
  const canvas = document.getElementById('canvas');
  const ctx = canvas.getContext('2d');

  // Constants
  const TILE_SIZE = 28;
  const VISION_SQUARE_TILES = 11;
  const HUD_HEIGHT = 48;

  // Speeds
  const PLAYER_WALK_SPEED = 4.2;
  const PLAYER_SPRINT_SPEED = 6.4;

  // Colors
  const COLOR_WALL = '#9b8a58';
  const COLOR_WALL_ACCENT = '#5f5230';
  const COLOR_FLOOR = '#3a3428';
  const COLOR_FLOOR_GRID = '#2c281e';
  const COLOR_DARK_VOID = '#060609';
  const COLOR_FLASHLIGHT = 'rgba(255, 240, 190, 0.12)';
  const COLOR_EXIT = '#28e678';
  const COLOR_CP_INACTIVE = '#3c8cf0';
  const COLOR_CP_ACTIVE = '#ffd73c';
  const COLOR_BLOOD_RED = '#dc1428';

  // Sound Synthesizer via Web Audio
  let audioCtx = null;
  function getAudioCtx() {
    if (!audioCtx) {
      audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    }
    if (audioCtx.state === 'suspended') {
      audioCtx.resume();
    }
    return audioCtx;
  }

  function playTone(freqs, durs, type = 'sine', vol = 0.3) {
    try {
      const ac = getAudioCtx();
      let startTime = ac.currentTime;
      for (let i = 0; i < freqs.length; ++i) {
        if (freqs[i] > 0) {
          const osc = ac.createOscillator();
          const gain = ac.createGain();
          osc.type = type;
          osc.frequency.setValueAtTime(freqs[i], startTime);
          gain.gain.setValueAtTime(vol, startTime);
          gain.gain.exponentialRampToValueAtTime(0.001, startTime + durs[i]);
          osc.connect(gain);
          gain.connect(ac.destination);
          osc.start(startTime);
          osc.stop(startTime + durs[i]);
        }
        startTime += durs[i];
      }
    } catch (e) {}
  }

  let lastHeartbeat = 0;
  function playHeartbeat(distFactor) {
    const now = Date.now();
    const interval = 350 + distFactor * 1200;
    if (now - lastHeartbeat >= interval) {
      lastHeartbeat = now;
      if (distFactor < 0.35) {
        playTone([70, 55, 0, 65, 50], [0.05, 0.04, 0.03, 0.05, 0.05], 'triangle', 0.6);
      } else {
        playTone([55, 45, 0, 50, 40], [0.08, 0.06, 0.05, 0.07, 0.08], 'sine', 0.45);
      }
    }
  }

  function playScreech() {
    playTone([440, 880, 1174, 1568, 2200, 800], [0.05, 0.05, 0.06, 0.08, 0.12, 0.15], 'sawtooth', 0.4);
  }

  function playCaught() {
    playTone([900, 450, 180, 60, 30], [0.10, 0.12, 0.20, 0.30, 0.30], 'sawtooth', 0.6);
  }

  function playEscaped() {
    playTone([261.6, 329.6, 392.0, 523.2, 659.2], [0.15, 0.15, 0.15, 0.20, 0.40], 'triangle', 0.4);
  }

  function playCheckpoint() {
    playTone([523.2, 659.2, 783.99], [0.10, 0.10, 0.25], 'sine', 0.35);
  }

  // Maze Generator
  class BacktrackerGenerator {
    constructor(loopProb = 0.12) {
      this.loopProb = loopProb;
    }

    generate(cols, rows) {
      const grid = Array.from({ length: rows }, () =>
        Array.from({ length: cols }, () => ({
          type: 'wall',
          visited: false,
          inVision: false,
          light: 0,
          checkpointActive: false
        }))
      );

      this.carve(1, 1, grid, cols, rows);
      this.addLoops(grid, cols, rows);

      const start = { x: 1, y: 1 };
      grid[start.y][start.x].type = 'floor';

      const exit = this.findFarthest(grid, cols, rows, start);
      grid[exit.y][exit.x].type = 'exit';

      this.placeCheckpoints(grid, cols, rows, start, exit);

      return { grid, start, exit, cols, rows };
    }

    carve(cx, cy, grid, cols, rows) {
      grid[cy][cx].type = 'floor';
      const dirs = [[0, -2], [0, 2], [-2, 0], [2, 0]];
      for (let i = 3; i > 0; i--) {
        const j = Math.floor(Math.random() * (i + 1));
        [dirs[i], dirs[j]] = [dirs[j], dirs[i]];
      }

      for (const [dx, dy] of dirs) {
        const nx = cx + dx, ny = cy + dy;
        if (nx > 0 && nx < cols - 1 && ny > 0 && ny < rows - 1) {
          if (grid[ny][nx].type === 'wall') {
            grid[cy + dy / 2][cx + dx / 2].type = 'floor';
            this.carve(nx, ny, grid, cols, rows);
          }
        }
      }
    }

    addLoops(grid, cols, rows) {
      for (let y = 1; y < rows - 1; y++) {
        for (let x = 1; x < cols - 1; x++) {
          if (grid[y][x].type === 'wall') {
            const h = grid[y][x - 1].type === 'floor' && grid[y][x + 1].type === 'floor';
            const v = grid[y - 1][x].type === 'floor' && grid[y + 1][x].type === 'floor';
            if ((h || v) && !(h && v) && Math.random() < this.loopProb) {
              grid[y][x].type = 'floor';
            }
          }
        }
      }
    }

    findFarthest(grid, cols, rows, start) {
      const dist = Array.from({ length: rows }, () => Array(cols).fill(-1));
      const q = [start];
      dist[start.y][start.x] = 0;
      let farthest = start, maxD = 0;

      while (q.length > 0) {
        const curr = q.shift();
        const d = dist[curr.y][curr.x];
        if (d > maxD) { maxD = d; farthest = curr; }
        for (const [dx, dy] of [[0, -1], [0, 1], [-1, 0], [1, 0]]) {
          const nx = curr.x + dx, ny = curr.y + dy;
          if (nx >= 0 && nx < cols && ny >= 0 && ny < rows) {
            if (grid[ny][nx].type === 'floor' && dist[ny][nx] === -1) {
              dist[ny][nx] = d + 1;
              q.push({ x: nx, y: ny });
            }
          }
        }
      }
      return farthest;
    }

    placeCheckpoints(grid, cols, rows, start, exit) {
      const dist = Array.from({ length: rows }, () => Array(cols).fill(-1));
      const q = [start];
      dist[start.y][start.x] = 0;

      while (q.length > 0) {
        const c = q.shift();
        const d = dist[c.y][c.x];
        for (const [dx, dy] of [[0, -1], [0, 1], [-1, 0], [1, 0]]) {
          const nx = c.x + dx, ny = c.y + dy;
          if (nx >= 0 && nx < cols && ny >= 0 && ny < rows) {
            if (grid[ny][nx].type !== 'wall' && dist[ny][nx] === -1) {
              dist[ny][nx] = d + 1;
              q.push({ x: nx, y: ny });
            }
          }
        }
      }

      const exitDist = dist[exit.y][exit.x];
      if (exitDist < 10) return;
      const t1 = Math.floor(exitDist * 0.45);
      const t2 = Math.floor(exitDist * 0.75);
      let best1 = null, best2 = null, diff1 = 999, diff2 = 999;

      for (let y = 1; y < rows - 1; y++) {
        for (let x = 1; x < cols - 1; x++) {
          if (grid[y][x].type === 'floor') {
            const d = dist[y][x];
            if (d > 0) {
              const df1 = Math.abs(d - t1);
              if (df1 < diff1) { diff1 = df1; best1 = { x, y }; }
              if (exitDist >= 24) {
                const df2 = Math.abs(d - t2);
                if (df2 < diff2) { diff2 = df2; best2 = { x, y }; }
              }
            }
          }
        }
      }

      if (best1) grid[best1.y][best1.x].type = 'checkpoint';
      if (best2 && (best2.x !== best1.x || best2.y !== best1.y)) grid[best2.y][best2.x].type = 'checkpoint';
    }
  }

  // Game Engine State
  const generator = new BacktrackerGenerator();
  let sessionLevels = [];
  let currentLevelIdx = 0;
  let activeGrid = null;
  let cols = 25, rows = 25;
  let startPos = { x: 1, y: 1 };
  let exitPos = { x: 1, y: 1 };
  let currentCheckpoint = { x: 1, y: 1 };

  let gameState = 'splash'; // splash, exploring, spotted, caught, gameover, escaped, victory
  let lives = 3;
  let difficulty = 'normal'; // easy, normal, hard

  const player = {
    x: 1, y: 1,
    facing: 'down',
    stamina: 1.0,
    sprinting: false,
    flashlight: true,
    noiseRadius: 0
  };

  let ghosts = [];

  function initSession() {
    sessionLevels = [
      generator.generate(25, 25),
      generator.generate(29, 29),
      generator.generate(31, 31)
    ];
    lives = 3;
    currentLevelIdx = 0;
    loadLevel(0);
  }

  function loadLevel(idx) {
    currentLevelIdx = idx;
    const lvl = sessionLevels[idx];
    cols = lvl.cols;
    rows = lvl.rows;
    startPos = lvl.start;
    exitPos = lvl.exit;
    currentCheckpoint = { ...startPos };

    // Deep copy grid so fog is fresh
    activeGrid = lvl.grid.map(row => row.map(cell => ({ ...cell, visited: false, inVision: false })));

    player.x = startPos.x;
    player.y = startPos.y;
    player.facing = 'down';
    player.stamina = 1.0;

    spawnGhosts(idx + 2);
    updateVision();
  }

  function spawnGhosts(count) {
    ghosts = [];
    const types = ['smiler', 'stalker', 'hound'];
    let spawned = 0, attempts = 0;
    while (spawned < count && attempts < 500) {
      attempts++;
      const gx = Math.floor(Math.random() * (cols - 2)) + 1;
      const gy = Math.floor(Math.random() * (rows - 2)) + 1;
      if (activeGrid[gy][gx].type !== 'wall') {
        const d = Math.hypot(gx - currentCheckpoint.x, gy - currentCheckpoint.y);
        if (d > 8) {
          ghosts.push({
            id: 100 + spawned,
            type: types[spawned % 3],
            x: gx, y: gy,
            nextX: gx, nextY: gy,
            facing: 'down',
            state: 'patrol',
            lastKnown: { x: -1, y: -1 },
            patrolTarget: { x: -1, y: -1 },
            timer: 0
          });
          spawned++;
        }
      }
    }
  }

  function respawnAtCheckpoint() {
    player.x = currentCheckpoint.x;
    player.y = currentCheckpoint.y;
    player.stamina = 1.0;
    spawnGhosts(currentLevelIdx + 2);
    updateVision();
    gameState = 'exploring';
  }

  function isWalkable(x, y) {
    if (x < 0 || x >= cols || y < 0 || y >= rows) return false;
    return activeGrid[y][x].type !== 'wall';
  }

  function hasLOS(from, to, maxRange) {
    const dx = to.x - from.x, dy = to.y - from.y;
    const dist = Math.hypot(dx, dy);
    if (dist > maxRange) return false;

    let x0 = from.x, y0 = from.y, x1 = to.x, y1 = to.y;
    const stepX = x0 < x1 ? 1 : -1, stepY = y0 < y1 ? 1 : -1;
    const absDx = Math.abs(x1 - x0), absDy = Math.abs(y1 - y0);
    let err = absDx - absDy;

    while (x0 !== x1 || y0 !== y1) {
      if ((x0 !== from.x || y0 !== from.y) && activeGrid[y0][x0].type === 'wall') {
        return false;
      }
      const e2 = 2 * err;
      if (e2 > -absDy) { err -= absDy; x0 += stepX; }
      if (e2 < absDx) { err += absDx; y0 += stepY; }
    }
    return true;
  }

  function updateVision() {
    if (!activeGrid) return;
    for (let y = 0; y < rows; y++) {
      for (let x = 0; x < cols; x++) {
        activeGrid[y][x].inVision = false;
      }
    }
    const pTileX = Math.round(player.x), pTileY = Math.round(player.y);
    const rad = Math.floor(VISION_SQUARE_TILES / 2);
    const minX = Math.max(0, pTileX - rad), maxX = Math.min(cols - 1, pTileX + rad);
    const minY = Math.max(0, pTileY - rad), maxY = Math.min(rows - 1, pTileY + rad);

    for (let y = minY; y <= maxY; y++) {
      for (let x = minX; x <= maxX; x++) {
        const c = activeGrid[y][x];
        c.inVision = true;
        c.visited = true;
        const d = Math.hypot(x - pTileX, y - pTileY);
        c.light = Math.max(0.15, 1.0 - d / (rad * 1.414));
      }
    }
  }

  // Input Handling
  const keys = {};
  window.addEventListener('keydown', e => {
    keys[e.key.toLowerCase()] = true;
    keys[e.code] = true;

    if (e.key === ' ' || e.key === 'Enter') {
      if (gameState === 'splash') {
        initSession();
        gameState = 'exploring';
      } else if (gameState === 'caught') {
        respawnAtCheckpoint();
      } else if (gameState === 'escaped') {
        if (currentLevelIdx < 2) {
          loadLevel(currentLevelIdx + 1);
          gameState = 'exploring';
        } else {
          gameState = 'victory';
          playEscaped();
        }
      } else if (gameState === 'gameover' || gameState === 'victory') {
        initSession();
        gameState = 'exploring';
      }
    } else if (e.key === 'r' || e.key === 'R') {
      if (gameState === 'caught') {
        loadLevel(currentLevelIdx);
        gameState = 'exploring';
      } else {
        initSession();
        gameState = 'exploring';
      }
    } else if (e.key === 'Escape') {
      gameState = 'splash';
    } else if (e.key === 'f' || e.key === 'F') {
      player.flashlight = !player.flashlight;
    } else if (e.key === 'Tab') {
      e.preventDefault();
      difficulty = difficulty === 'easy' ? 'normal' : (difficulty === 'normal' ? 'hard' : 'easy');
    } else if (e.key === '1') {
      difficulty = 'easy';
    } else if (e.key === '2') {
      difficulty = 'normal';
    } else if (e.key === '3') {
      difficulty = 'hard';
    }
  });

  window.addEventListener('keyup', e => {
    keys[e.key.toLowerCase()] = false;
    keys[e.code] = false;
  });

  canvas.addEventListener('click', e => {
    const rect = canvas.getBoundingClientRect();
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;
    const mx = (e.clientX - rect.left) * scaleX;
    const my = (e.clientY - rect.top) * scaleY;

    if (gameState === 'splash') {
      // Check if clicked on difficulty buttons
      // curY on splash was ~448, buttons are at y: ~434..456
      if (my >= 430 && my <= 465) {
        if (mx >= 350 && mx <= 445) { difficulty = 'easy'; return; }
        if (mx >= 446 && mx <= 545) { difficulty = 'normal'; return; }
        if (mx >= 546 && mx <= 645) { difficulty = 'hard'; return; }
      }
      initSession();
      gameState = 'exploring';
    } else if (gameState === 'caught') {
      respawnAtCheckpoint();
    } else if (gameState === 'escaped') {
      if (currentLevelIdx < 2) {
        loadLevel(currentLevelIdx + 1);
        gameState = 'exploring';
      } else {
        gameState = 'victory';
        playEscaped();
      }
    } else if (gameState === 'gameover' || gameState === 'victory') {
      initSession();
      gameState = 'exploring';
    }
  });

  // Movement & Collision
  function canMoveTo(nx, ny) {
    const r = 0.30;
    const minX = Math.floor(nx - r + 0.5), maxX = Math.floor(nx + r + 0.5);
    const minY = Math.floor(ny - r + 0.5), maxY = Math.floor(ny + r + 0.5);
    for (let ty = minY; ty <= maxY; ty++) {
      for (let tx = minX; tx <= maxX; tx++) {
        if (!isWalkable(tx, ty)) return false;
      }
    }
    return true;
  }

  function updatePlayer(dt) {
    const sprinting = (keys['shift'] || keys['shiftleft'] || keys['shiftright']) && player.stamina > 0.05;
    player.sprinting = sprinting;

    let dx = 0, dy = 0;
    if (keys['w'] || keys['arrowup']) { dy -= 1; player.facing = 'up'; }
    if (keys['s'] || keys['arrowdown']) { dy += 1; player.facing = 'down'; }
    if (keys['a'] || keys['arrowleft']) { dx -= 1; player.facing = 'left'; }
    if (keys['d'] || keys['arrowright']) { dx += 1; player.facing = 'right'; }

    const isMoving = dx !== 0 || dy !== 0;
    if (isMoving && sprinting) {
      player.stamina = Math.max(0, player.stamina - dt * 0.35);
    } else {
      player.stamina = Math.min(1.0, player.stamina + dt * 0.20);
    }

    player.noiseRadius = isMoving ? (sprinting ? 3.2 : 1.4) : 0;
    const speed = (sprinting ? PLAYER_SPRINT_SPEED : PLAYER_WALK_SPEED) * dt;

    if (dx !== 0) {
      const nx = player.x + Math.sign(dx) * speed;
      if (canMoveTo(nx, player.y)) player.x = nx;
    }
    if (dy !== 0) {
      const ny = player.y + Math.sign(dy) * speed;
      if (canMoveTo(player.x, ny)) player.y = ny;
    }

    updateVision();
  }

  function updateGhosts(dt) {
    let anyChasing = false;
    let minDist = 999;
    const pTile = { x: Math.round(player.x), y: Math.round(player.y) };

    const basePatrol = difficulty === 'easy' ? 1.6 : (difficulty === 'normal' ? 2.0 : 2.5);
    const baseChase = difficulty === 'easy' ? 2.5 : (difficulty === 'normal' ? 3.2 : 4.0);
    const maxLos = difficulty === 'easy' ? 3.5 : (difficulty === 'normal' ? 4.0 : 4.5);

    for (const g of ghosts) {
      g.timer += dt;
      const gTile = { x: Math.round(g.x), y: Math.round(g.y) };
      const d = Math.hypot(player.x - g.x, player.y - g.y);
      if (d < minDist) minDist = d;

      // Sensory: Blind past maxLos!
      let seesPlayer = false;
      if (d <= maxLos) {
        seesPlayer = hasLOS(gTile, pTile, maxLos);
      }

      if (seesPlayer) {
        if (g.state !== 'chase') playScreech();
        g.state = 'chase';
        g.lastKnown = { ...pTile };
        g.timer = 0;
      } else if (g.state === 'chase') {
        g.state = 'investigate';
        g.timer = 0;
      }

      // Hearing
      if (player.noiseRadius > 0 && d <= player.noiseRadius && g.state === 'patrol') {
        g.state = 'investigate';
        g.lastKnown = { ...pTile };
        g.timer = 0;
      }

      let speed = basePatrol;
      let target = pTile;

      if (g.state === 'chase') {
        anyChasing = true;
        speed = g.type === 'stalker' ? baseChase * 1.08 : baseChase;
        target = pTile;
      } else if (g.state === 'investigate') {
        speed = basePatrol * 1.15;
        target = g.lastKnown;
        if ((gTile.x === g.lastKnown.x && gTile.y === g.lastKnown.y) || g.timer > 2.8) {
          g.state = 'cooldown';
          g.timer = 0;
        }
      } else if (g.state === 'cooldown') {
        speed = basePatrol * 0.6;
        if (g.timer > 2.0) { g.state = 'patrol'; g.timer = 0; }
      } else {
        // Patrol wander
        if (gTile.x === g.patrolTarget.x && gTile.y === g.patrolTarget.y || g.patrolTarget.x === -1) {
          const opts = [];
          for (const [dx, dy] of [[0, -1], [0, 1], [-1, 0], [1, 0]]) {
            const nx = gTile.x + dx, ny = gTile.y + dy;
            if (isWalkable(nx, ny)) opts.push({ x: nx, y: ny });
          }
          if (opts.length > 0) g.patrolTarget = opts[Math.floor(Math.random() * opts.length)];
        }
        target = g.patrolTarget;
      }

      // Step towards target
      const distNext = Math.hypot(g.nextX - g.x, g.nextY - g.y);
      if (distNext < 0.08 || !isWalkable(g.nextX, g.nextY)) {
        g.x = g.nextX; g.y = g.nextY;
        // Simple 1-step BFS
        const nxt = getNextStep(gTile, target);
        g.nextX = nxt.x; g.nextY = nxt.y;
      }

      const dx = g.nextX - g.x, dy = g.nextY - g.y;
      const len = Math.hypot(dx, dy);
      if (len > 0.001) {
        const step = Math.min(len, speed * dt);
        g.x += (dx / len) * step;
        g.y += (dy / len) * step;
      }

      // Kill check
      if (d < 0.62) {
        lives--;
        playCaught();
        gameState = lives > 0 ? 'caught' : 'gameover';
        return;
      }
    }

    if (gameState === 'exploring' || gameState === 'spotted') {
      gameState = anyChasing ? 'spotted' : 'exploring';
    }

    if (minDist < 12.0) {
      playHeartbeat(Math.min(1.0, minDist / 12.0));
    }
  }

  function getNextStep(from, to) {
    if (from.x === to.x && from.y === to.y) return from;
    const q = [from];
    const visited = Array.from({ length: rows }, () => Array(cols).fill(false));
    const parent = Array.from({ length: rows }, () => Array(cols).fill(null));
    visited[from.y][from.x] = true;
    let found = false;

    while (q.length > 0) {
      const c = q.shift();
      if (c.x === to.x && c.y === to.y) { found = true; break; }
      for (const [dx, dy] of [[0, -1], [0, 1], [-1, 0], [1, 0]]) {
        const nx = c.x + dx, ny = c.y + dy;
        if (isWalkable(nx, ny) && !visited[ny][nx]) {
          visited[ny][nx] = true;
          parent[ny][nx] = c;
          q.push({ x: nx, y: ny });
        }
      }
    }
    if (!found) return from;
    let curr = to;
    while (parent[curr.y][curr.x] && (parent[curr.y][curr.x].x !== from.x || parent[curr.y][curr.x].y !== from.y)) {
      curr = parent[curr.y][curr.x];
    }
    return curr;
  }

  function checkCollisions() {
    const pt = { x: Math.round(player.x), y: Math.round(player.y) };
    if (isWalkable(pt.x, pt.y)) {
      const cell = activeGrid[pt.y][pt.x];
      if (cell.type === 'checkpoint' && !cell.checkpointActive) {
        cell.checkpointActive = true;
        currentCheckpoint = { ...pt };
        playCheckpoint();
      }
    }

    const dExit = Math.hypot(player.x - exitPos.x, player.y - exitPos.y);
    if (dExit < 0.65) {
      gameState = 'escaped';
      playEscaped();
    }
  }

  // Camera & Screen transform
  let camX = 1, camY = 1;
  function updateCamera(dt) {
    camX += (player.x - camX) * Math.min(1.0, 10.0 * dt);
    camY += (player.y - camY) * Math.min(1.0, 10.0 * dt);
  }

  function worldToScreen(wx, wy) {
    const sx = canvas.width / 2 + (wx - camX) * TILE_SIZE;
    const sy = HUD_HEIGHT + (canvas.height - HUD_HEIGHT) / 2 + (wy - camY) * TILE_SIZE;
    return { x: sx, y: sy };
  }

  // Rendering
  let pulseTimer = 0, flickerTimer = 0;
  function render() {
    ctx.fillStyle = COLOR_DARK_VOID;
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    if (gameState === 'splash') {
      renderSplash();
      return;
    }

    if (!activeGrid) return;

    // 1. Render Tiles
    for (let y = 0; y < rows; y++) {
      for (let x = 0; x < cols; x++) {
        const c = activeGrid[y][x];
        if (!c.visited) continue;

        const p = worldToScreen(x, y);
        const rect = { x: p.x - TILE_SIZE / 2, y: p.y - TILE_SIZE / 2, w: TILE_SIZE, h: TILE_SIZE };
        const light = c.inVision ? c.light : 0.08;

        if (c.type === 'wall') {
          ctx.fillStyle = COLOR_WALL;
          ctx.globalAlpha = light;
          ctx.fillRect(rect.x, rect.y, rect.w, rect.h);
          ctx.strokeStyle = COLOR_WALL_ACCENT;
          ctx.lineWidth = 1;
          ctx.strokeRect(rect.x, rect.y, rect.w, rect.h);
        } else if (c.type === 'floor') {
          ctx.fillStyle = COLOR_FLOOR;
          ctx.globalAlpha = light;
          ctx.fillRect(rect.x, rect.y, rect.w, rect.h);
          ctx.strokeStyle = COLOR_FLOOR_GRID;
          ctx.lineWidth = 1;
          ctx.strokeRect(rect.x, rect.y, rect.w, rect.h);
        } else if (c.type === 'checkpoint') {
          ctx.fillStyle = COLOR_FLOOR;
          ctx.globalAlpha = light;
          ctx.fillRect(rect.x, rect.y, rect.w, rect.h);
          if (c.inVision) {
            const act = c.checkpointActive;
            const pulse = 0.6 + 0.4 * Math.sin(pulseTimer * 3.5);
            ctx.fillStyle = act ? COLOR_CP_ACTIVE : COLOR_CP_INACTIVE;
            ctx.globalAlpha = pulse;
            ctx.beginPath();
            ctx.arc(p.x, p.y, TILE_SIZE * 0.38, 0, Math.PI * 2);
            ctx.fill();
            ctx.strokeStyle = '#fff';
            ctx.stroke();

            ctx.fillStyle = act ? '#000' : '#fff';
            ctx.globalAlpha = 1;
            ctx.font = 'bold 8px Consolas';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(act ? 'SAFE' : 'SAVE', p.x, p.y);
          }
        } else if (c.type === 'exit') {
          if (c.inVision) {
            const pulse = 0.7 + 0.3 * Math.sin(pulseTimer * 4.0);
            ctx.fillStyle = COLOR_EXIT;
            ctx.globalAlpha = pulse;
            ctx.fillRect(rect.x, rect.y, rect.w, rect.h);
            ctx.fillStyle = '#fff';
            ctx.globalAlpha = 1;
            ctx.font = 'bold 8px Consolas';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText('EXIT', p.x, p.y);
          }
        }
      }
    }
    ctx.globalAlpha = 1.0;

    // 2. Render Ghosts
    for (const g of ghosts) {
      const p = worldToScreen(g.x, g.y);
      const inVision = Math.abs(g.x - player.x) <= VISION_SQUARE_TILES / 2 && Math.abs(g.y - player.y) <= VISION_SQUARE_TILES / 2;
      const r = TILE_SIZE * 0.42;

      if (inVision) {
        ctx.fillStyle = g.type === 'smiler' ? '#0f0f14' : (g.type === 'stalker' ? '#231419' : '#2d2314');
        ctx.strokeStyle = g.state === 'chase' ? '#dc1414' : '#444';
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        ctx.arc(p.x, p.y, r, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();
      }

      if (inVision || g.state === 'chase') {
        const eyeCol = g.state === 'chase' ? '#ff1e1e' : (g.type === 'smiler' ? '#f5f5ff' : (g.type === 'stalker' ? '#ff8c00' : '#b4dc28'));
        ctx.fillStyle = eyeCol;
        ctx.beginPath();
        ctx.arc(p.x - r * 0.38, p.y - r * 0.2, 2.5, 0, Math.PI * 2);
        ctx.arc(p.x + r * 0.38, p.y - r * 0.2, 2.5, 0, Math.PI * 2);
        ctx.fill();

        if (g.type === 'smiler' && inVision) {
          ctx.strokeStyle = eyeCol;
          ctx.lineWidth = 1.5;
          ctx.beginPath();
          ctx.arc(p.x, p.y - r * 0.1, r * 0.5, 0.2 * Math.PI, 0.8 * Math.PI);
          ctx.stroke();
        }
      }
    }

    // 3. Render Player
    const pSc = worldToScreen(player.x, player.y);
    const pr = TILE_SIZE * 0.38;

    if (player.flashlight) {
      const beamLen = TILE_SIZE * 1.8, spread = TILE_SIZE * 0.7;
      ctx.fillStyle = 'rgba(255, 245, 200, 0.14)';
      ctx.beginPath();
      ctx.moveTo(pSc.x, pSc.y);
      if (player.facing === 'up') {
        ctx.lineTo(pSc.x - spread, pSc.y - beamLen);
        ctx.lineTo(pSc.x + spread, pSc.y - beamLen);
      } else if (player.facing === 'down') {
        ctx.lineTo(pSc.x - spread, pSc.y + beamLen);
        ctx.lineTo(pSc.x + spread, pSc.y + beamLen);
      } else if (player.facing === 'left') {
        ctx.lineTo(pSc.x - beamLen, pSc.y - spread);
        ctx.lineTo(pSc.x - beamLen, pSc.y + spread);
      } else {
        ctx.lineTo(pSc.x + beamLen, pSc.y - spread);
        ctx.lineTo(pSc.x + beamLen, pSc.y + spread);
      }
      ctx.closePath();
      ctx.fill();
    }

    ctx.fillStyle = '#f5d71e';
    ctx.strokeStyle = '#1e1e14';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.arc(pSc.x, pSc.y, pr, 0, Math.PI * 2);
    ctx.fill();
    ctx.stroke();

    ctx.fillStyle = '#1e232d';
    let vx = pSc.x, vy = pSc.y;
    if (player.facing === 'up') vy -= pr * 0.4;
    else if (player.facing === 'down') vy += pr * 0.4;
    else if (player.facing === 'left') vx -= pr * 0.4;
    else vx += pr * 0.4;
    ctx.beginPath();
    ctx.arc(vx, vy, pr * 0.4, 0, Math.PI * 2);
    ctx.fill();

    // 4. Shroud Outside Square Vision
    const visPx = VISION_SQUARE_TILES * TILE_SIZE;
    const vRect = { x: pSc.x - visPx / 2, y: pSc.y - visPx / 2, w: visPx, h: visPx };

    ctx.save();
    ctx.fillStyle = COLOR_DARK_VOID;
    ctx.beginPath();
    ctx.rect(0, 0, canvas.width, canvas.height);
    ctx.rect(vRect.x + vRect.w, vRect.y, -vRect.w, vRect.h); // Cutout
    ctx.fill();
    ctx.restore();

    ctx.strokeStyle = '#5a5037';
    ctx.lineWidth = 2;
    ctx.strokeRect(vRect.x, vRect.y, vRect.w, vRect.h);

    if (player.flashlight) {
      const flicker = 0.85 + 0.15 * Math.sin(flickerTimer * 12.0) * Math.cos(flickerTimer * 7.0);
      ctx.fillStyle = COLOR_FLASHLIGHT;
      ctx.globalAlpha = 0.12 * flicker;
      ctx.fillRect(vRect.x, vRect.y, vRect.w, vRect.h);
      ctx.globalAlpha = 1.0;
    }

    // 5. HUD
    renderHUD();

    // 6. Overlays
    renderStateOverlay();
  }

  function renderHUD() {
    ctx.fillStyle = 'rgba(14, 14, 18, 0.9)';
    ctx.fillRect(0, 0, canvas.width, HUD_HEIGHT);
    ctx.strokeStyle = '#282832';
    ctx.beginPath();
    ctx.moveTo(0, HUD_HEIGHT); ctx.lineTo(canvas.width, HUD_HEIGHT);
    ctx.stroke();

    ctx.font = 'bold 12px Consolas';
    ctx.textAlign = 'left';
    ctx.fillStyle = '#b4b4b4';
    ctx.fillText(`LEVEL ${currentLevelIdx + 1} / 3 : THE LABYRINTH`, 16, 20);

    const diffCol = difficulty === 'easy' ? '#50dc64' : (difficulty === 'normal' ? '#f0c828' : '#f03232');
    ctx.fillStyle = diffCol;
    ctx.fillText(`[${difficulty.toUpperCase()}]`, 200, 20);

    let livesStr = 'LIVES: ';
    for (let i = 0; i < 3; i++) livesStr += (i < lives) ? '[●] ' : '[ ] ';
    ctx.fillStyle = lives > 1 ? '#f0d228' : '#e62828';
    ctx.fillText(livesStr, 16, 38);

    // Stamina
    ctx.strokeStyle = '#444';
    ctx.strokeRect(265, 14, 105, 14);
    ctx.fillStyle = player.stamina > 0.25 ? '#dcb428' : '#dc3232';
    ctx.fillRect(267, 16, (105 - 4) * player.stamina, 10);
    ctx.fillStyle = '#fff';
    ctx.font = 'bold 10px Consolas';
    ctx.fillText('STAMINA [SHIFT]', 265, 40);

    // Pulse
    let minDist = 999;
    for (const g of ghosts) {
      const d = Math.hypot(player.x - g.x, player.y - g.y);
      if (d < minDist) minDist = d;
    }
    const bpm = Math.floor(70 + Math.max(0, (12 - minDist) * 10));
    ctx.fillStyle = bpm > 110 ? COLOR_BLOOD_RED : '#64c878';
    ctx.fillText(`PULSE: ${bpm} BPM`, 420, 28);

    if (gameState === 'spotted') {
      ctx.fillStyle = COLOR_BLOOD_RED;
      ctx.fillText('!! ENTITY SPOTTED !!', 580, 28);
    } else {
      ctx.fillStyle = '#8c8c8c';
      ctx.fillText('[R] RESTART | [ESC] MENU', 580, 28);
    }
  }

  function renderSplash() {
    ctx.fillStyle = '#08080c';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    ctx.textAlign = 'center';
    ctx.font = 'bold 24px Consolas';
    ctx.fillStyle = '#f5d73c';
    ctx.fillText('PACMAN MAZE: SURVIVAL IN THE DARK', canvas.width / 2, 50);

    ctx.font = '12px Consolas';
    ctx.fillStyle = '#9696a0';
    ctx.fillText('-- 3-LEVEL PROCEDURAL ESCAPE PROTOCOL --', canvas.width / 2, 85);

    const box = { x: 37, y: 110, w: 710, h: 460 };
    ctx.fillStyle = 'rgba(14, 15, 20, 0.9)';
    ctx.fillRect(box.x, box.y, box.w, box.h);
    ctx.strokeStyle = '#46412d';
    ctx.lineWidth = 2;
    ctx.strokeRect(box.x, box.y, box.w, box.h);

    ctx.textAlign = 'left';
    let curY = box.y + 30;
    const rules = [
      ['1. OBJECTIVE', 'Find the unmarked green EXIT portal hidden in each maze.', '#32dc78'],
      ['2. 3 LEVELS', '3 unique dynamic mazes generated for this session. Escape all 3 to win.', '#dcb428'],
      ['3. 4-TILE FOG', 'Entities are completely BLIND past ~4 tiles! Use distance & darkness to hide.', '#64d2ff'],
      ['4. 3 LIVES', 'You have 3 lives for the entire session. Die thrice = Session Over.', '#f04646'],
      ['5. CHECKPOINTS', 'Step onto glowing blue beacons to save a safe respawn point in the level.', '#3c8cf0'],
      ['6. STEALTH', 'Walking is silent. Sprinting [SHIFT] is fast but emits sound within 3.2 tiles.', '#f0a028']
    ];

    for (const [tag, desc, col] of rules) {
      ctx.font = 'bold 11px Consolas';
      ctx.fillStyle = col;
      ctx.fillText(tag, box.x + 24, curY);
      ctx.font = '10px Consolas';
      ctx.fillStyle = '#d7d7d7';
      ctx.fillText(desc, box.x + 150, curY);
      curY += 26;
    }

    curY += 10;
    ctx.strokeStyle = '#32323c';
    ctx.beginPath();
    ctx.moveTo(box.x + 24, curY); ctx.lineTo(box.x + box.w - 24, curY);
    ctx.stroke();
    curY += 22;

    ctx.font = 'bold 11px Consolas';
    ctx.fillStyle = '#ffb43c';
    ctx.fillText('ENTITY EYE & BEHAVIOR INTEL:', box.x + 24, curY);
    curY += 22;

    const intel = [
      ['WHITE + SMILE', 'The Smiler', 'Creepy grin & glowing eyes. Lurks in the dark.', '#f5f5ff'],
      ['AMBER GLOW', 'The Stalker', 'Cautious hunter. Navigates corridors smoothly.', '#ff8c00'],
      ['YELLOW-GREEN', 'The Hound', 'Canine predator. Very sensitive to sprint footsteps.', '#b4dc28'],
      ['BLOOD RED', 'IN PURSUIT', 'Entity locked sight on you! Turn corners to break LOS!', '#ff1e1e']
    ];

    for (const [eye, name, desc, col] of intel) {
      ctx.font = 'bold 10px Consolas';
      ctx.fillStyle = col;
      ctx.fillText(eye, box.x + 36, curY);
      ctx.fillStyle = '#e6e6e6';
      ctx.fillText(name, box.x + 150, curY);
      ctx.font = '10px Consolas';
      ctx.fillStyle = '#aaaab4';
      ctx.fillText(desc, box.x + 260, curY);
      curY += 22;
    }

    curY += 10;
    ctx.strokeStyle = '#32323c';
    ctx.beginPath();
    ctx.moveTo(box.x + 24, curY); ctx.lineTo(box.x + box.w - 24, curY);
    ctx.stroke();
    curY += 24;

    ctx.font = 'bold 11px Consolas';
    ctx.fillStyle = '#eee';
    ctx.fillText('SELECT DIFFICULTY [Press TAB or 1/2/3]:', box.x + 24, curY);

    const diffs = [
      { key: 'easy', label: '[1] EASY', col: '#50dc64' },
      { key: 'normal', label: '[2] NORMAL', col: '#f0c828' },
      { key: 'hard', label: '[3] HARD', col: '#f03232' }
    ];

    let bx = box.x + 320;
    for (const d of diffs) {
      const active = difficulty === d.key;
      ctx.strokeStyle = active ? d.col : '#50505a';
      ctx.lineWidth = active ? 2 : 1;
      ctx.strokeRect(bx, curY - 14, 90, 22);
      ctx.fillStyle = active ? d.col : '#82828c';
      ctx.textAlign = 'center';
      ctx.fillText(d.label, bx + 45, curY);
      bx += 100;
    }

    // Start prompt
    ctx.textAlign = 'center';
    const pulse = 0.6 + 0.4 * Math.sin(pulseTimer * 4.0);
    ctx.font = 'bold 15px Consolas';
    ctx.fillStyle = `rgba(255, 220, 60, ${pulse})`;
    ctx.fillText('PRESS [SPACE] OR [ENTER] TO BEGIN SESSION', canvas.width / 2, canvas.height - 85);

    ctx.font = '11px Consolas';
    ctx.fillStyle = '#82828c';
    ctx.fillText('[WASD] Move  |  [SHIFT] Sprint  |  [F] Flashlight  |  [TAB] Difficulty  |  [R] Re-roll Session', canvas.width / 2, canvas.height - 48);
  }

  function renderStateOverlay() {
    ctx.textAlign = 'center';
    if (gameState === 'caught') {
      ctx.fillStyle = 'rgba(140, 10, 10, 0.82)';
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      ctx.font = 'bold 30px Consolas';
      ctx.fillStyle = '#fff';
      ctx.fillText('YOU WERE CONSUMED', canvas.width / 2, canvas.height / 2 - 60);

      ctx.font = '15px Consolas';
      ctx.fillStyle = '#ffd778';
      ctx.fillText(`Lives Remaining: ${lives} / 3`, canvas.width / 2, canvas.height / 2 - 10);

      ctx.fillStyle = '#fff';
      ctx.fillText('Press [SPACE] to Respawn at Last Checkpoint', canvas.width / 2, canvas.height / 2 + 30);
      ctx.font = '13px Consolas';
      ctx.fillStyle = '#ccc';
      ctx.fillText('Press [R] to Restart Level  |  [ESC] Exit to Menu', canvas.width / 2, canvas.height / 2 + 65);
    } else if (gameState === 'gameover') {
      ctx.fillStyle = 'rgba(30, 5, 5, 0.92)';
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      ctx.font = 'bold 30px Consolas';
      ctx.fillStyle = '#f02828';
      ctx.fillText('SESSION OVER', canvas.width / 2, canvas.height / 2 - 60);

      ctx.font = '15px Consolas';
      ctx.fillStyle = '#ddd';
      ctx.fillText('All 3 lives were lost in the labyrinth.', canvas.width / 2, canvas.height / 2 - 10);

      ctx.fillStyle = '#ffd73c';
      ctx.fillText('Press [SPACE] or [R] to Start a New 3-Level Session', canvas.width / 2, canvas.height / 2 + 30);
      ctx.font = '13px Consolas';
      ctx.fillStyle = '#bbb';
      ctx.fillText('Press [ESC] for Title Screen', canvas.width / 2, canvas.height / 2 + 65);
    } else if (gameState === 'escaped') {
      ctx.fillStyle = 'rgba(15, 80, 35, 0.85)';
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      ctx.font = 'bold 30px Consolas';
      ctx.fillStyle = '#fff';
      ctx.fillText(`LEVEL ${currentLevelIdx + 1} CLEARED!`, canvas.width / 2, canvas.height / 2 - 50);

      ctx.font = '15px Consolas';
      ctx.fillStyle = '#ffffc8';
      ctx.fillText(`Preparing Next Level: ${currentLevelIdx + 2} / 3`, canvas.width / 2, canvas.height / 2);

      ctx.fillStyle = '#fff';
      ctx.fillText('Press [SPACE] to Enter Next Level', canvas.width / 2, canvas.height / 2 + 45);
    } else if (gameState === 'victory') {
      ctx.fillStyle = 'rgba(10, 45, 75, 0.90)';
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      ctx.font = 'bold 30px Consolas';
      ctx.fillStyle = '#ffe13c';
      ctx.fillText('YOU CONQUERED THE LABYRINTH!', canvas.width / 2, canvas.height / 2 - 60);

      ctx.font = '15px Consolas';
      ctx.fillStyle = '#fff';
      ctx.fillText('All 3 dynamic levels successfully escaped.', canvas.width / 2, canvas.height / 2 - 10);

      ctx.fillStyle = '#64dcff';
      ctx.fillText('Press [SPACE] or [R] to Start a New Session', canvas.width / 2, canvas.height / 2 + 30);
      ctx.font = '13px Consolas';
      ctx.fillStyle = '#bbb';
      ctx.fillText('Press [ESC] for Title Screen', canvas.width / 2, canvas.height / 2 + 65);
    }
  }

  // Main Loop
  let lastTime = performance.now();
  function gameLoop(now) {
    const dt = Math.min(0.05, (now - lastTime) / 1000.0);
    lastTime = now;

    pulseTimer += dt * 2.0;
    flickerTimer += dt * 1.5;

    if (gameState === 'exploring' || gameState === 'spotted') {
      updatePlayer(dt);
      updateGhosts(dt);
      checkCollisions();
      updateCamera(dt);
    }

    render();
    requestAnimationFrame(gameLoop);
  }

  requestAnimationFrame(gameLoop);
})();
