/**
 * LeArm Protocol Layer
 * Source: LeArm_ESP32_Arduino_factory250512 firmware
 *
 * ── Serial ──────────────────────────────────────────────────────────────────
 * 9600 baud, 8N1 — confirmed from Serial.begin(9600) in .ino
 *
 * ── Packet format (PC → ARM) ─────────────────────────────────────────────────
 * [0x55] [0x55] [len] [cmd] [...data]
 * len = 1 (len byte) + 1 (cmd byte) + data.length
 *
 * ── Servo ID → physical joint mapping ───────────────────────────────────────
 * Confirmed from Pwmservo.cpp pwm_servos[] init (reverse order) +
 * knot_run() which calls set_duty(6-id, ...):
 *
 *   ID 1 → Gripper       (SERVO_1 pin, duty clamped 500–1500 by firmware)
 *   ID 2 → Wrist Roll    (SERVO_2 pin)
 *   ID 3 → Elbow Lower   (SERVO_3 pin)
 *   ID 4 → Elbow Upper   (SERVO_4 pin)
 *   ID 5 → Shoulder      (SERVO_5 pin)
 *   ID 6 → Base Rotate   (SERVO_6 pin)
 *
 * ── Duty range ───────────────────────────────────────────────────────────────
 * PWM mode: 500–2500 µs (standard servo pulse width)
 * Values below 500 cause a hard jump on init; firmware clamps above 2500.
 * ID 1 (gripper) further clamped to 500–1500 by knot_run().
 *
 * ── Software offsets ─────────────────────────────────────────────────────────
 * APP_SERVO_OFFSET_SET (cmd 10): sets runtime trim for one servo
 * APP_SERVO_OFFSET_DOWNLOAD (cmd 11): writes offset to ESP32 flash
 * Range: −125 to +125 µs (int8_t)
 */

const FRAME_HEADER = 0x55;

const CMD = {
  VERSION_QUERY:        1,
  SERVO_OFFSET_READ:    2,
  MULT_SERVO_MOVE:      3,
  COORDINATE_SET:       4,
  ACTION_GROUP_RUN:     6,
  ACTION_GROUP_STOP:    7,
  ACTION_GROUP_ERASE:   8,
  SERVO_OFFSET_SET:     10,
  SERVO_OFFSET_SAVE:    11,
  SERVOS_RESET:         12,
  SERVOS_READ:          13,
  ACTION_DOWNLOAD:      25,
};

// Home positions in µs (PWM pulsewidth), from Config.h TYPE_PWM_SERVO
const HOME_POSITIONS = { 1: 1500, 2: 1500, 3: 644, 4: 511, 5: 1255, 6: 770 };

const SERVO_COUNT = 6;

// ─── Packet builder ───────────────────────────────────────────────────────────
function buildPacket(cmd, data = []) {
  const len = 1 + 1 + data.length; // len byte + cmd + payload
  const buf = Buffer.alloc(2 + 1 + 1 + data.length);
  buf[0] = FRAME_HEADER;
  buf[1] = FRAME_HEADER;
  buf[2] = len;
  buf[3] = cmd;
  data.forEach((b, i) => { buf[4 + i] = b & 0xFF; });
  return buf;
}

// ─── Commands ─────────────────────────────────────────────────────────────────

/**
 * Move one or more servos simultaneously.
 * @param {Array<{id, duty}>} servos
 * @param {number} duration ms
 */
function moveServos(servos, duration = 500) {
  const data = [servos.length, duration & 0xFF, (duration >> 8) & 0xFF];
  for (const s of servos) {
    data.push(s.id & 0xFF);
    data.push(s.duty & 0xFF);
    data.push((s.duty >> 8) & 0xFF);
  }
  return buildPacket(CMD.MULT_SERVO_MOVE, data);
}

function moveServo(id, duty, duration = 500) {
  return moveServos([{ id, duty }], duration);
}

function resetHome() {
  return buildPacket(CMD.SERVOS_RESET, []);
}

function queryVersion() {
  return buildPacket(CMD.VERSION_QUERY, []);
}

function readServos() {
  return buildPacket(CMD.SERVOS_READ, []);
}

function stopAction() {
  return buildPacket(CMD.ACTION_GROUP_STOP, []);
}

/**
 * Set software trim offset for one servo.
 * @param {number} id servo ID 1–6
 * @param {number} offset -125 to +125 µs
 */
function setOffset(id, offset) {
  offset = Math.max(-125, Math.min(125, Math.round(offset)));
  return buildPacket(CMD.SERVO_OFFSET_SET, [id & 0xFF, offset & 0xFF]);
}

/**
 * Write current offset to ESP32 flash (persists across power cycles).
 * @param {number} id servo ID 1–6
 */
function saveOffset(id) {
  return buildPacket(CMD.SERVO_OFFSET_SAVE, [id & 0xFF]);
}

// ─── Response parser ──────────────────────────────────────────────────────────
class ResponseParser {
  constructor() { this._buf = Buffer.alloc(0); }

  feed(chunk) {
    this._buf = Buffer.concat([this._buf, chunk]);
    const packets = [];
    while (this._buf.length >= 4) {
      if (this._buf[0] !== FRAME_HEADER || this._buf[1] !== FRAME_HEADER) {
        this._buf = this._buf.slice(1);
        continue;
      }
      const len = this._buf[2];
      const total = 2 + len;
      if (this._buf.length < total) break;
      const cmd  = this._buf[3];
      const data = this._buf.slice(4, total);
      packets.push({ cmd, data });
      this._buf = this._buf.slice(total);
    }
    return packets;
  }
}

/**
 * Parse SERVOS_READ response into [{id, duty}] array.
 * Response payload after cmd byte: [id1, duty1_lo, duty1_hi, id2, ...]
 */
function parseServosRead(data) {
  const servos = [];
  for (let i = 0; i < SERVO_COUNT; i++) {
    const off = i * 3;
    servos.push({ id: data[off], duty: data[off + 1] | (data[off + 2] << 8) });
  }
  return servos;
}

function parseVersion(data) {
  return { servoType: data[0] === 1 ? 'PWM' : 'Serial Bus', version: data[1] };
}

module.exports = {
  CMD, HOME_POSITIONS, SERVO_COUNT, FRAME_HEADER,
  buildPacket,
  moveServos, moveServo, resetHome, queryVersion, readServos, stopAction,
  setOffset, saveOffset,
  ResponseParser, parseServosRead, parseVersion,
};
