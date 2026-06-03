/**
 * LeArm Web Controller — Server
 * 
 * Run: node src/server.js [--port 3000] [--serial /dev/ttyUSB0]
 * 
 * The arm communicates at 9600 baud over USB serial.
 * Make sure the arm is in PC mode (press button once after power-on;
 * LED should blink slowly at 1s intervals).
 */

const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const path = require('path');

const {
  CMD,
  HOME_POSITIONS,
  moveServos,
  moveServo,
  resetHome,
  queryVersion,
  readServos,
  stopAction,
  ResponseParser,
  parseServosRead,
  parseVersion,
} = require('./protocol');

// ─── Config (override via env or CLI) ────────────────────────────────────────
const PORT        = parseInt(process.env.PORT || '3000', 10);
const SERIAL_PATH = process.env.SERIAL_PATH || '/dev/ttyUSB0';
const BAUD_RATE   = 9600;

// ─── App setup ────────────────────────────────────────────────────────────────
const app = express();
const server = http.createServer(app);
const io = new Server(server, {
  cors: { origin: '*' }
});

app.use(express.static(path.join(__dirname, '../public')));
app.use(express.json());

// ─── Serial port ─────────────────────────────────────────────────────────────
let port = null;
let serialConnected = false;
const parser = new ResponseParser();

async function connectSerial(serialPath = SERIAL_PATH) {
  try {
    const { SerialPort } = await import('serialport');

    if (port && port.isOpen) {
      await new Promise((res) => port.close(res));
    }

    port = new SerialPort({
      path: serialPath,
      baudRate: BAUD_RATE,
      autoOpen: false,
    });

    port.open((err) => {
      if (err) {
        console.error(`[serial] Failed to open ${serialPath}:`, err.message);
        serialConnected = false;
        io.emit('serial:status', { connected: false, error: err.message, path: serialPath });
        return;
      }
      serialConnected = true;
      console.log(`[serial] Connected to ${serialPath} @ ${BAUD_RATE} baud`);
      io.emit('serial:status', { connected: true, path: serialPath });

      // After connecting, query firmware version
      setTimeout(() => sendToArm(queryVersion()), 300);
    });

    port.on('data', (chunk) => {
      const packets = parser.feed(chunk);
      for (const pkt of packets) {
        handleArmResponse(pkt);
      }
    });

    port.on('error', (err) => {
      console.error('[serial] Error:', err.message);
      serialConnected = false;
      io.emit('serial:status', { connected: false, error: err.message });
    });

    port.on('close', () => {
      serialConnected = false;
      console.log('[serial] Port closed');
      io.emit('serial:status', { connected: false, path: serialPath });
    });

  } catch (err) {
    console.error('[serial] Import/init error:', err.message);
    io.emit('serial:status', { connected: false, error: err.message });
  }
}

function sendToArm(buf) {
  if (!port || !port.isOpen) {
    console.warn('[serial] Not connected — ignoring command');
    return false;
  }
  port.write(buf, (err) => {
    if (err) console.error('[serial] Write error:', err.message);
  });
  return true;
}

// ─── Response handler ─────────────────────────────────────────────────────────
function handleArmResponse({ cmd, data }) {
  switch (cmd) {
    case CMD.VERSION_QUERY: {
      const info = parseVersion(data);
      console.log('[arm] Firmware version:', info);
      io.emit('arm:version', info);
      // After version, read current servo positions
      setTimeout(() => sendToArm(readServos()), 100);
      break;
    }
    case CMD.SERVOS_READ: {
      const servos = parseServosRead(data);
      io.emit('arm:positions', servos);
      break;
    }
    default:
      io.emit('arm:raw', { cmd, data: Array.from(data) });
  }
}

// ─── REST: list available serial ports ───────────────────────────────────────
app.get('/api/ports', async (req, res) => {
  try {
    const { SerialPort } = await import('serialport');
    const ports = await SerialPort.list();
    res.json({ ports });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.get('/api/status', (req, res) => {
  res.json({ connected: serialConnected, path: SERIAL_PATH });
});

// ─── Socket.IO events ─────────────────────────────────────────────────────────
io.on('connection', (socket) => {
  console.log('[ws] Client connected:', socket.id);

  // Send current state to new client
  socket.emit('serial:status', { connected: serialConnected, path: SERIAL_PATH });

  // ── Connect / disconnect serial ──────────────────────────────────────────
  socket.on('serial:connect', ({ path: p }) => {
    console.log('[ws] serial:connect →', p);
    connectSerial(p || SERIAL_PATH);
  });

  socket.on('serial:disconnect', () => {
    if (port && port.isOpen) port.close();
  });

  // ── Arm commands ─────────────────────────────────────────────────────────

  /**
   * Move one servo
   * { id: 1-6, duty: 0-2500, duration: ms }
   */
  socket.on('arm:move_servo', ({ id, duty, duration = 500 }) => {
    sendToArm(moveServo(id, duty, duration));
  });

  /**
   * Move multiple servos simultaneously
   * { servos: [{id, duty}], duration: ms }
   */
  socket.on('arm:move_servos', ({ servos, duration = 500 }) => {
    sendToArm(moveServos(servos, duration));
  });

  /**
   * Reset all servos to home
   */
  socket.on('arm:reset', () => {
    sendToArm(resetHome());
  });

  /**
   * Stop any running action
   */
  socket.on('arm:stop', () => {
    sendToArm(stopAction());
  });

  /**
   * Read all servo positions
   */
  socket.on('arm:read', () => {
    sendToArm(readServos());
  });

  /**
   * Raw packet send (for debugging)
   * { bytes: [0x55, 0x55, ...] }
   */
  socket.on('arm:raw_send', ({ bytes }) => {
    sendToArm(Buffer.from(bytes));
  });

  socket.on('disconnect', () => {
    console.log('[ws] Client disconnected:', socket.id);
  });
});

// ─── Start ────────────────────────────────────────────────────────────────────
server.listen(PORT, '0.0.0.0', () => {
  console.log(`\n╔══════════════════════════════════════╗`);
  console.log(`║   LeArm Controller  →  :${PORT}          ║`);
  console.log(`╚══════════════════════════════════════╝`);
  console.log(`  Serial: ${SERIAL_PATH} @ ${BAUD_RATE} baud`);
  console.log(`  Open browser: http://localhost:${PORT}\n`);
  // Try to auto-connect on startup
  connectSerial(SERIAL_PATH);
});
