# LeArm Web Controller

Web-based GUI to control the **Hiwonder LeArm / xArm1S** (6-DOF) over USB serial from a Raspberry Pi (or any Linux machine).

Built with Node.js, Express, Socket.IO, and the `serialport` library. Protocol derived directly from the ESP32 firmware source.

---

## Hardware Setup

1. **Power on the arm** via USB-C (or battery)
2. **Connect USB** from Pi to the arm's controller board
3. **Switch to PC mode** — press the button on the controller board **once** after power-on  
   → LED blinks slowly (1 second on / 1 second off) = PC mode confirmed  
   → Button cycles: App → **PC** → PS2 → Freeplay → App

The arm appears as `/dev/ttyUSB0` or `/dev/ttyACM0` on Linux.

---

## Software Setup (Raspberry Pi)

```bash
# 1. Clone or copy the project
cd ~
# (copy learm-controller folder here)

# 2. Install Node.js if not already installed
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt-get install -y nodejs

# 3. Install dependencies
cd learm-controller
npm install

# 4. Give your user access to the serial port
sudo usermod -a -G dialout $USER
# Log out and back in for this to take effect

# 5. Start the server
npm start
```

Then open a browser (on the Pi or any machine on the same network):

```
http://<raspberry-pi-ip>:3000
```

---

## Configuration

Override defaults via environment variables:

```bash
PORT=8080 SERIAL_PATH=/dev/ttyACM0 npm start
```

| Variable      | Default          | Description                   |
|---------------|------------------|-------------------------------|
| `PORT`        | `3000`           | HTTP server port              |
| `SERIAL_PATH` | `/dev/ttyUSB0`   | Serial device path            |

---

## Protocol Reference

Derived from `LeArm_ESP32_Arduino_factory250512` firmware source.

**Serial:** 9600 baud, 8N1  
**Packet format (PC → Arm):**
```
[0x55] [0x55] [len] [cmd] [...data]
```
`len` = 1 (len byte) + 1 (cmd) + data.length

**Key commands:**

| CMD ID | Name                 | Payload                                      |
|--------|----------------------|----------------------------------------------|
| 1      | VERSION_QUERY        | none                                         |
| 3      | MULT_SERVO_MOVE      | `count, time_lo, time_hi, [id, duty_lo, duty_hi] x N` |
| 6      | ACTION_GROUP_RUN     | `group_index, times_lo, times_hi`            |
| 7      | ACTION_GROUP_STOP    | none                                         |
| 12     | SERVOS_RESET         | none                                         |
| 13     | SERVOS_READ          | none                                         |

**Servo positions:** 0–2500 duty units (PWM mode), maps to 0–240° mechanical range  
**Home positions:** S1=770, S2=1500, S3=644, S4=511, S5=1255, S6=1500

---

## Project Structure

```
learm-controller/
├── src/
│   ├── server.js       ← Express + Socket.IO server
│   └── protocol.js     ← Packet builder / parser (from firmware)
├── public/
│   └── index.html      ← Web GUI (single file, no build step)
├── package.json
└── README.md
```

---

## Future: OpenCV Integration

The server is structured to support a Python subprocess for vision:

```bash
# Planned flow:
# 1. Python script captures webcam → detects objects → publishes coordinates via stdin/socket
# 2. Node server receives coordinates → emits to browser OR sends direct arm commands
# 3. Browser shows live camera feed via WebSocket + canvas
```

The Pi 4B with 2GB RAM is plenty for Node.js + OpenCV simultaneously.

---

## Development

```bash
# Auto-restart on file changes (requires devDependencies)
npm run dev
```
