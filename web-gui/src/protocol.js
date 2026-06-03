/**
 * LeArm Protocol Layer
 * 
 * Based on firmware source: LeArm_ESP32_Arduino_factory250512
 * 
 * USB Serial: 9600 baud, 8N1
 * 
 * PC → ARM packet format:
 *   [0x55] [0x55] [length] [cmd] [...data]
 *   where length = number of bytes AFTER the two headers (so includes length byte itself + cmd + data)
 * 
 * ARM → PC response same framing.
 * 
 * NOTE: Arm boots in BLE/App mode. Press the physical button ONCE to enter PC mode
 * (LED blinks 1s on / 1s off). Button press cycles: App → PC → PS2 → Freeplay → App
 */

// ─── Command IDs (from PC_BLE_CTL.hpp AppFunctionStatus enum) ───────────────
const CMD = {
  VERSION_QUERY:        1,   // Query firmware version → responds with [cmd, servo_type, version]
  SERVO_OFFSET_READ:    2,   // Read all 6 servo offsets
  MULT_SERVO_MOVE:      3,   // Move one or more servos
  COORDINATE_SET:       4,   // Cartesian XYZ move (requires kinematics)
  ACTION_GROUP_RUN:     6,   // Run a saved action group
  ACTION_GROUP_STOP:    7,   // Stop running action group
  ACTION_GROUP_ERASE:   8,   // Erase all saved action groups
  SERVO_OFFSET_SET:     10,  // Set offset for one servo
  SERVO_OFFSET_SAVE:    11,  // Save offsets to flash
  SERVOS_RESET:         12,  // Move all servos to home position
  SERVOS_READ:          13,  // Read current position of all 6 servos
  ACTION_DOWNLOAD:      25,  // Download an action group frame
};

// ─── Servo home positions (PWM duty, from Config.h TYPE_PWM_SERVO) ───────────
const HOME_POSITIONS = {
  1: 770,
  2: 1500,
  3: 644,
  4: 511,
  5: 1255,
  6: 1500,
};

const SERVO_COUNT = 6;
const FRAME_HEADER = 0x55;

// ─── Packet builder ──────────────────────────────────────────────────────────

/**
 * Build a packet to send to the arm over USB serial.
 * Format: 0x55 0x55 [len] [cmd] [...data]
 * len = 1 (for len byte itself) + 1 (cmd) + data.length
 */
function buildPacket(cmd, data = []) {
  const len = 1 + 1 + data.length; // len field + cmd + payload
  const buf = Buffer.alloc(3 + data.length);
  buf[0] = FRAME_HEADER;
  buf[1] = FRAME_HEADER;
  buf[2] = len;
  buf[3] = cmd;
  for (let i = 0; i < data.length; i++) {
    buf[4 + i] = data[i] & 0xFF;
  }
  return buf;
}

// ─── Command builders ────────────────────────────────────────────────────────

/**
 * Move one or more servos simultaneously.
 * @param {Array<{id: number, duty: number}>} servos - servo IDs (1-6) and duty (0-2500)
 * @param {number} duration - time in ms to complete move
 */
function moveServos(servos, duration = 500) {
  const count = servos.length;
  const data = [
    count,
    duration & 0xFF,
    (duration >> 8) & 0xFF,
  ];
  for (const s of servos) {
    data.push(s.id & 0xFF);
    data.push(s.duty & 0xFF);
    data.push((s.duty >> 8) & 0xFF);
  }
  return buildPacket(CMD.MULT_SERVO_MOVE, data);
}

/**
 * Move a single servo.
 */
function moveServo(id, duty, duration = 500) {
  return moveServos([{ id, duty }], duration);
}

/**
 * Reset all servos to home position.
 */
function resetHome(duration = 1000) {
  return buildPacket(CMD.SERVOS_RESET, []);
}

/**
 * Query firmware version.
 */
function queryVersion() {
  return buildPacket(CMD.VERSION_QUERY, []);
}

/**
 * Read all servo positions.
 */
function readServos() {
  return buildPacket(CMD.SERVOS_READ, []);
}

/**
 * Stop any running action group.
 */
function stopAction() {
  return buildPacket(CMD.ACTION_GROUP_STOP, []);
}

// ─── Response parser ─────────────────────────────────────────────────────────

class ResponseParser {
  constructor() {
    this._buf = Buffer.alloc(0);
  }

  /**
   * Feed incoming bytes into the parser.
   * Returns an array of parsed packets (each is { cmd, data: Buffer }).
   */
  feed(chunk) {
    this._buf = Buffer.concat([this._buf, chunk]);
    const packets = [];

    while (this._buf.length >= 4) {
      // Look for 0x55 0x55 header
      if (this._buf[0] !== FRAME_HEADER || this._buf[1] !== FRAME_HEADER) {
        // Shift one byte and retry
        this._buf = this._buf.slice(1);
        continue;
      }

      const len = this._buf[2]; // includes len byte + cmd + data
      const totalExpected = 2 + len; // 2 headers + len field value

      if (this._buf.length < totalExpected) break; // wait for more data

      const cmd = this._buf[3];
      const data = this._buf.slice(4, totalExpected);
      packets.push({ cmd, data });
      this._buf = this._buf.slice(totalExpected);
    }

    return packets;
  }
}

/**
 * Parse a SERVOS_READ response.
 * Returns array of { id, duty } for all 6 servos.
 */
function parseServosRead(data) {
  const servos = [];
  // Response: [cmd, id1, duty1_lo, duty1_hi, id2, duty2_lo, duty2_hi, ...]
  // data starts after cmd byte (already stripped by parser)
  for (let i = 0; i < SERVO_COUNT; i++) {
    const offset = i * 3;
    const id = data[offset];
    const duty = data[offset + 1] | (data[offset + 2] << 8);
    servos.push({ id, duty });
  }
  return servos;
}

/**
 * Parse a VERSION_QUERY response.
 */
function parseVersion(data) {
  return {
    servoType: data[0] === 1 ? 'PWM' : 'Serial Bus',
    version: data[1],
  };
}

module.exports = {
  CMD,
  HOME_POSITIONS,
  SERVO_COUNT,
  buildPacket,
  moveServos,
  moveServo,
  resetHome,
  queryVersion,
  readServos,
  stopAction,
  ResponseParser,
  parseServosRead,
  parseVersion,
};
