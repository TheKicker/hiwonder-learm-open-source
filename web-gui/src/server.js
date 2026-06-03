/**
 * LeArm Web Controller — Server
 *
 * Serial: 9600 baud confirmed from firmware (Serial.begin(9600) in .ino)
 *
 * Arm must be in PC mode:
 *   Power on → press button once → LED blinks 1s on/1s off = PC mode
 */

const express = require('express');
const http    = require('http');
const { Server } = require('socket.io');
const path    = require('path');

const {
  CMD,
  moveServos, moveServo, resetHome, queryVersion, readServos, stopAction,
  setOffset, saveOffset,
  ResponseParser, parseServosRead, parseVersion,
} = require('./protocol');

const PORT        = parseInt(process.env.PORT || '3000', 10);
const SERIAL_PATH = process.env.SERIAL_PATH || '/dev/ttyUSB0';
const BAUD_RATE   = 9600; // confirmed from firmware source Serial.begin(9600)

const app    = express();
const server = http.createServer(app);
const io     = new Server(server, { cors: { origin: '*' } });

app.use(express.static(path.join(__dirname, '../public')));
app.use(express.json());

// ─── Serial ───────────────────────────────────────────────────────────────────
let port = null;
let serialConnected = false;
const parser = new ResponseParser();

async function connectSerial(serialPath = SERIAL_PATH) {
  try {
    const { SerialPort } = await import('serialport');
    if (port && port.isOpen) await new Promise(res => port.close(res));

    port = new SerialPort({ path: serialPath, baudRate: BAUD_RATE, autoOpen: false });

    port.open(err => {
      if (err) {
        console.error(`[serial] Failed to open ${serialPath}:`, err.message);
        serialConnected = false;
        io.emit('serial:status', { connected: false, error: err.message, path: serialPath });
        return;
      }
      serialConnected = true;
      console.log(`[serial] Connected → ${serialPath} @ ${BAUD_RATE} baud`);
      io.emit('serial:status', { connected: true, path: serialPath });
      // Query version then read positions
      setTimeout(() => sendToArm(queryVersion()), 300);
    });

    port.on('data', chunk => {
      for (const pkt of parser.feed(chunk)) handleResponse(pkt);
    });

    port.on('error', err => {
      console.error('[serial] Error:', err.message);
      serialConnected = false;
      io.emit('serial:status', { connected: false, error: err.message });
    });

    port.on('close', () => {
      serialConnected = false;
      console.log('[serial] Closed');
      io.emit('serial:status', { connected: false, path: serialPath });
    });
  } catch (err) {
    console.error('[serial] Import error:', err.message);
    io.emit('serial:status', { connected: false, error: err.message });
  }
}

function sendToArm(buf) {
  if (!port || !port.isOpen) { console.warn('[serial] Not connected'); return false; }
  port.write(buf, err => { if (err) console.error('[serial] Write error:', err.message); });
  return true;
}

// ─── Response handler ─────────────────────────────────────────────────────────
function handleResponse({ cmd, data }) {
  switch (cmd) {
    case CMD.VERSION_QUERY:
      const info = parseVersion(data);
      console.log('[arm] FW:', info);
      io.emit('arm:version', info);
      setTimeout(() => sendToArm(readServos()), 150);
      break;
    case CMD.SERVOS_READ:
      io.emit('arm:positions', parseServosRead(data));
      break;
    default:
      io.emit('arm:raw', { cmd, data: Array.from(data) });
  }
}

// ─── REST ─────────────────────────────────────────────────────────────────────
app.get('/api/ports', async (req, res) => {
  try {
    const { SerialPort } = await import('serialport');
    res.json({ ports: await SerialPort.list() });
  } catch (err) { res.status(500).json({ error: err.message }); }
});

app.get('/api/status', (_req, res) => {
  res.json({ connected: serialConnected, path: SERIAL_PATH });
});

// ─── Socket events ────────────────────────────────────────────────────────────
io.on('connection', socket => {
  console.log('[ws] Client:', socket.id);
  socket.emit('serial:status', { connected: serialConnected, path: SERIAL_PATH });

  socket.on('serial:connect',    ({ path: p }) => connectSerial(p || SERIAL_PATH));
  socket.on('serial:disconnect', ()            => { if (port && port.isOpen) port.close(); });

  // Single servo — throttled calls arrive here
  socket.on('arm:move_servo', ({ id, duty, duration = 500 }) => {
    sendToArm(moveServo(id, duty, duration));
  });

  // Multi-servo simultaneous move
  socket.on('arm:move_servos', ({ servos, duration = 500 }) => {
    sendToArm(moveServos(servos, duration));
  });

  // Reset all to home
  socket.on('arm:reset', () => sendToArm(resetHome()));

  // Stop motion
  socket.on('arm:stop', () => sendToArm(stopAction()));

  // Read positions from arm
  socket.on('arm:read', () => sendToArm(readServos()));

  // Set software offset for one servo (−125 to +125 µs)
  socket.on('arm:set_offset', ({ id, offset }) => {
    sendToArm(setOffset(id, offset));
  });

  // Save all offsets to arm's flash
  socket.on('arm:save_offsets', () => {
    for (let id = 1; id <= 6; id++) {
      sendToArm(saveOffset(id));
    }
    console.log('[arm] Offsets saved to flash');
  });

  // Raw bytes (debug)
  socket.on('arm:raw_send', ({ bytes }) => sendToArm(Buffer.from(bytes)));

  socket.on('disconnect', () => console.log('[ws] Disconnected:', socket.id));
});

// ─── Start ────────────────────────────────────────────────────────────────────
server.listen(PORT, '0.0.0.0', () => {
  console.log(`\n╔═══════════════════════════════════╗`);
  console.log(`║  LeArm Controller  →  :${PORT}       ║`);
  console.log(`╚═══════════════════════════════════╝`);
  console.log(`  Serial: ${SERIAL_PATH} @ ${BAUD_RATE} baud`);
  console.log(`  Open:   http://localhost:${PORT}\n`);
  connectSerial(SERIAL_PATH);
});
