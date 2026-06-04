/**
 * LeArm Web Controller — Server
 * Cross-platform: Windows (COM*), macOS (/dev/tty.usbserial-*), Linux (/dev/ttyUSB*)
 *
 * Serial: 9600 baud confirmed from firmware (Serial.begin(9600) in .ino)
 *
 * Usage:
 *   node src/server.js
 *   PORT=8080 node src/server.js
 *   SERIAL_PATH=COM6 node src/server.js
 *   SERIAL_PATH=/dev/ttyUSB0 node src/server.js
 */

const express = require('express');
const http    = require('http');
const { Server } = require('socket.io');
const path    = require('path');
const os      = require('os');

const {
  CMD,
  moveServos, moveServo, resetHome, queryVersion, readServos, stopAction,
  setOffset, saveOffset,
  ResponseParser, parseServosRead, parseVersion,
} = require('./protocol');

const PORT      = parseInt(process.env.PORT || '3000', 10);
const BAUD_RATE = 9600;

// ─── Platform-aware serial port default ───────────────────────────────────────
function getDefaultSerialPath() {
  if (process.env.SERIAL_PATH) return process.env.SERIAL_PATH;
  const platform = os.platform();
  if (platform === 'win32')  return 'COM6';
  if (platform === 'darwin') return '/dev/tty.usbserial-0001';
  return '/dev/ttyUSB0'; // Linux / Raspberry Pi
}

const SERIAL_PATH = getDefaultSerialPath();

const app    = express();
const server = http.createServer(app);
const io     = new Server(server, { cors: { origin: '*' } });

app.use(express.static(path.join(__dirname, '../public')));
app.use(express.json());

// ─── Serial ───────────────────────────────────────────────────────────────────
let port = null;
let serialConnected = false;
let currentPath = SERIAL_PATH;
const parser = new ResponseParser();

async function connectSerial(serialPath) {
  serialPath = serialPath || currentPath;
  currentPath = serialPath;

  try {
    const { SerialPort } = await import('serialport');

    if (port && port.isOpen) {
      await new Promise(res => port.close(res));
      port = null;
    }

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
      setTimeout(() => sendToArm(queryVersion()), 300);
    });

    port.on('data', chunk => {
      for (const pkt of parser.feed(chunk)) handleResponse(pkt);
    });

    port.on('error', err => {
      console.error('[serial] Error:', err.message);
      serialConnected = false;
      io.emit('serial:status', { connected: false, error: err.message, path: serialPath });
    });

    port.on('close', () => {
      serialConnected = false;
      console.log('[serial] Closed');
      io.emit('serial:status', { connected: false, path: serialPath });
    });

  } catch (err) {
    console.error('[serial] Error:', err.message);
    io.emit('serial:status', { connected: false, error: err.message, path: serialPath });
  }
}

function sendToArm(buf) {
  if (!port || !port.isOpen) {
    console.warn('[serial] Not connected — ignoring command');
    return false;
  }
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
    const ports = await SerialPort.list();
    res.json({ ports, platform: os.platform(), defaultPath: SERIAL_PATH });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.get('/api/status', (_req, res) => {
  res.json({ connected: serialConnected, path: currentPath, platform: os.platform() });
});

// ─── Socket events ────────────────────────────────────────────────────────────
io.on('connection', socket => {
  console.log('[ws] Client connected:', socket.id);
  socket.emit('serial:status', { connected: serialConnected, path: currentPath });

  socket.on('serial:connect',    ({ path: p }) => connectSerial(p));
  socket.on('serial:disconnect', () => { if (port && port.isOpen) port.close(); });

  socket.on('arm:move_servo',  ({ id, duty, duration = 500 }) => {
    sendToArm(moveServo(id, duty, duration));
  });
  socket.on('arm:move_servos', ({ servos, duration = 500 }) => {
    sendToArm(moveServos(servos, duration));
  });
  // Reset to home = explicit 1500 on all 6 servos.
  // CMD.SERVOS_RESET uses hardcoded factory values from Config.h (770, 644, 511 etc.)
  // which don't match a mechanically-zeroed arm. Always send 1500 explicitly.
  socket.on('arm:reset', () => {
    const home = [1,2,3,4,5,6].map(id => ({ id, duty: 1500 }));
    sendToArm(moveServos(home, 1000));
  });
  socket.on('arm:stop',        () => sendToArm(stopAction()));

  // Factory reset — Hiwonder original Config.h values, NOT your mechanical zero
  // Only correct if arm is assembled per Hiwonder's original instructions
  socket.on('arm:factory_reset', () => sendToArm(resetHome()));
  socket.on('arm:read',        () => sendToArm(readServos()));

  socket.on('arm:set_offset',  ({ id, offset }) => sendToArm(setOffset(id, offset)));
  socket.on('arm:save_offsets', () => {
    for (let id = 1; id <= 6; id++) sendToArm(saveOffset(id));
    console.log('[arm] Offsets saved to flash');
  });

  socket.on('arm:raw_send', ({ bytes }) => sendToArm(Buffer.from(bytes)));
  socket.on('disconnect',   () => console.log('[ws] Client disconnected:', socket.id));
});

// ─── Start ────────────────────────────────────────────────────────────────────
server.listen(PORT, '0.0.0.0', () => {
  const platform = { win32: 'Windows', darwin: 'macOS', linux: 'Linux' }[os.platform()] || os.platform();
  console.log(`\n╔══════════════════════════════════════════╗`);
  console.log(`║   LeArm Controller  →  :${PORT}             ║`);
  console.log(`╚══════════════════════════════════════════╝`);
  console.log(`  Platform: ${platform}`);
  console.log(`  Serial:   ${SERIAL_PATH} @ ${BAUD_RATE} baud`);
  console.log(`  Open:     http://localhost:${PORT}`);
  console.log(`\n  Override: SERIAL_PATH=COM3 node src/server.js\n`);
  connectSerial(SERIAL_PATH);
});
