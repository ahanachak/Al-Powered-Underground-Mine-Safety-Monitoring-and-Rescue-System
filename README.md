# Mine Rescue Rover

## AI-Powered Underground Mine Safety Monitoring and Rescue System

A Smart India Hackathon (SIH) project designed to provide **remote situational awareness, environmental monitoring, communication, obstacle detection, and low-bandwidth mapping** for underground mine-rescue operations.

The system combines an **Arduino UNO Q rover**, environmental sensors, LoRa communication, an optical camera, and a YDLIDAR X2. The architecture separates high-bandwidth visual data from safety-critical environmental telemetry so that important hazard information can continue to reach the operator even when high-bandwidth communication is unavailable or unreliable.

---

## 1. Project Objectives

The rover is intended to assist rescue teams by providing:

- Live optical video from the underground environment.
- Temperature, humidity, and atmospheric-pressure monitoring.
- Gas/smoke hazard indication using the MQ-2 sensor.
- Long-range LoRa communication for critical telemetry.
- Local real-time LiDAR-based obstacle detection.
- Low-resolution LiDAR mapping transmitted to a remote dashboard.
- A browser-based monitoring dashboard for the rescue operator.
- A communication architecture that prioritizes environmental safety data over non-critical mapping traffic.

---

## 2. System Architecture

```text
                         UNDERGROUND ROVER
┌───────────────────────────────────────────────────────────────┐
│                         Arduino UNO Q                         │
│                                                               │
│  ┌──────────────┐       ┌──────────────┐                    │
│  │   BME280     │       │    MQ-2      │                    │
│  │ Temp/Hum/    │       │ Gas Digital  │                    │
│  │ Pressure     │       │ Alert        │                    │
│  └──────┬───────┘       └──────┬───────┘                    │
│         │                      │                              │
│         └──────────┬───────────┘                              │
│                    │                                          │
│              Safety Telemetry                                 │
│                    │                                          │
│                    ▼                                          │
│               ┌─────────┐                                    │
│               │  RA-02  │                                    │
│               │  LoRa   │                                    │
│               └────┬────┘                                    │
│                    │                                          │
│  ┌──────────────┐  │       ┌────────────────┐                │
│  │ USB Webcam   │──┼──────►│ Wi-Fi / HTTP   │──────┐         │
│  └──────────────┘  │       └────────────────┘      │         │
│                    │                               │         │
│  ┌──────────────┐  │                               │         │
│  │ YDLIDAR X2   │──┘                               │         │
│  │              │                                  │         │
│  │ Local real-  │                                  │         │
│  │ time obstacle│                                  │         │
│  │ detection    │                                  │         │
│  └──────────────┘                                  │         │
└────────────────────────────────────────────────────┼─────────┘
                                                     │
                                                     │ Wi-Fi
                                                     ▼
                                            ┌─────────────────┐
                                            │ Laptop / Ground │
                                            │ Station        │
                                            │                 │
                                            │ Flask Dashboard │
                                            └────────┬────────┘
                                                     │
                                  ┌──────────────────┼──────────────┐
                                  │                  │              │
                                  ▼                  ▼              ▼
                             Live Camera       Environmental    LiDAR Map
                                                Telemetry
```

### LoRa path

```text
BME280 + MQ-2
      │
      ▼
 Arduino UNO Q
      │
      ▼
    RA-02
      │
   LoRa 433 MHz
      │
      ▼
    RA-02
      │
      ▼
    ESP32
      │
     USB
      │
      ▼
    Laptop
      │
      ▼
 Flask Dashboard
```

### Camera path

```text
USB Webcam
    │
    ▼
 Arduino UNO Q
    │
 OpenCV / Flask
    │
    ▼
 Wi-Fi
    │
    ▼
 Laptop Browser
```

### LiDAR path

```text
YDLIDAR X2
     │
     ▼
 Arduino UNO Q
     │
     ├── Real-time local obstacle detection
     │
     └── Low-resolution map data
              │
              ▼
           RA-02 LoRa
              │
              ▼
          Receiver ESP32
              │
             USB
              │
              ▼
          Flask Dashboard
```

---

## 3. Communication Priority

The system is deliberately designed so that LiDAR mapping does not interfere with safety-critical environmental telemetry.

### Priority levels

| Priority | Data | Purpose |
|---|---|---|
| HIGH | MQ-2 gas alert | Immediate hazard indication |
| HIGH | BME280 telemetry | Environmental monitoring |
| LOW | LiDAR mapping | Remote situational mapping |
| LOCAL | LiDAR obstacle detection | Immediate rover navigation/safety |

### Important design principle

**Obstacle detection is performed locally on the UNO Q.**

It therefore does not depend on LoRa bandwidth.

LiDAR mapping is treated as lower-priority traffic. If LoRa bandwidth becomes constrained, environmental telemetry is transmitted first and LiDAR mapping can be delayed, reduced, or dropped.

The system should avoid building an indefinitely growing queue of stale LiDAR data.

---

## 4. Hardware

### Main controller

- Arduino UNO Q

### Environmental sensors

- BME280
  - Temperature
  - Humidity
  - Atmospheric pressure
- MQ-2
  - Digital gas/smoke alert output

### Communication

- RA-02 LoRa modules
- ESP32 receiver

### Perception

- USB optical webcam
- YDLIDAR X2

### Power

- 2 × 18650 Li-ion cells for the motor/power subsystem
- Separate USB-C power bank for the UNO Q

The UNO Q is intentionally powered separately from the 2-cell motor battery system to reduce the risk of motor/power transients reaching the controller's logic power.

---

## 5. Pin Configuration

### BME280 → Arduino UNO Q

| BME280 | UNO Q |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| SDA | A4 / D18 |
| SCL | A5 / D19 |

I²C address:

```text
0x76
```

### MQ-2 → Arduino UNO Q

| MQ-2 | UNO Q |
|---|---|
| DO | D3 |
| GND | GND |
| VCC | Module supply |

Only the **digital output** is currently used.

The MQ-2 analog output is intentionally not used in the current configuration.

The digital output is treated as a **relative gas/smoke hazard indication**, not as a quantitative gas concentration measurement.

### RA-02 LoRa transmitter → UNO Q

| RA-02 | UNO Q |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| DIO0 | D2 |
| NSS / CS | D10 |
| MOSI | D11 |
| MISO | D12 |
| SCK | D13 |
| RESET | D9 |

Current LoRa frequency:

```text
433 MHz
```

Current radio settings:

```text
Spreading Factor: 7
Bandwidth:        125 kHz
Coding Rate:      4/5
CRC:              Enabled
TX Power:         17
```

### RA-02 → ESP32 receiver

| RA-02 | ESP32 |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| SCK | GPIO 18 |
| MISO | GPIO 19 |
| MOSI | GPIO 23 |
| CS | GPIO 5 |
| RST | GPIO 14 |
| DIO0 | GPIO 26 |

---

## 6. Environmental Telemetry

The UNO Q periodically reads:

```text
Temperature
Humidity
Pressure
Gas status
```

The current LoRa packet format is:

```text
TEMP=27.2,HUM=61.4,PRESS=1004.7,GAS=SAFE
```

Possible gas states include:

```text
SAFE
ALERT
```

The ESP32 receiver converts the received telemetry into JSON:

```json
{
  "temp": 27.2,
  "humidity": 61.4,
  "pressure": 1004.7,
  "gas": "SAFE",
  "rssi": -48,
  "snr": 9.5
}
```

This JSON is sent through USB serial to the laptop.

---

## 7. Ground Station Dashboard

The laptop runs a Flask web application.

Current dashboard URL:

```text
http://127.0.0.1:5001
```

The dashboard provides:

- Live camera feed
- Temperature
- Humidity
- Atmospheric pressure
- Gas status
- LoRa connection status
- RSSI
- SNR
- Last telemetry update
- LiDAR visualization area

The dashboard reads telemetry from:

```text
/api/data
```

and polls the endpoint periodically.

---

## 8. Camera System

The USB webcam is connected to the UNO Q.

The UNO Q uses Linux/OpenCV to capture the camera and exposes an MJPEG stream through Flask.

Current camera endpoint:

```text
http://10.161.47.161:5001/video_feed
```

The laptop dashboard embeds this stream.

The camera therefore does **not** consume LoRa bandwidth.

---

## 9. LiDAR System

The rover uses a YDLIDAR X2.

The X2 currently connects through a USB-to-serial interface using a Silicon Labs CP210x bridge.

When detected by Linux, the device appears as:

```text
/dev/ttyUSB0
```

The UNO Q Linux system has already been observed detecting:

```text
cp210x converter detected
cp210x converter now attached to ttyUSB0
```

The current USB hub has shown intermittent detection/disconnection behavior with the LiDAR adapter. Direct USB connection has been used for testing.

### Planned LiDAR processing

The LiDAR data will have two separate purposes:

1. **Local obstacle detection**
   - Fast
   - Performed on the UNO Q
   - Does not depend on LoRa

2. **Remote mapping**
   - Lower resolution
   - Lower update rate
   - Downsampled before transmission
   - Packetized for LoRa
   - Reconstructed on the receiver/dashboard

Raw high-rate LiDAR data should not be continuously transmitted over LoRa.

---

## 10. LoRa LiDAR Packetization

Because LoRa has substantially less bandwidth than a direct USB/Wi-Fi connection, LiDAR mapping data will be reduced before transmission.

A conceptual packet structure is:

```text
MAP,<scan_id>,<part>,<total_parts>,<data>
```

For example:

```text
MAP,102,1,3,...
MAP,102,2,3,...
MAP,102,3,3,...
```

The receiver can use:

- Scan ID
- Packet number
- Total packet count

to reconstruct a complete low-resolution scan.

Environmental telemetry must remain independent of this queue and retain transmission priority.

---

## 11. Software Stack

### Rover

- Arduino UNO Q
- Arduino sketch
- BME280 library
- LoRa library
- Linux/Debian environment
- OpenCV
- Flask
- USB serial communication

### Receiver

- ESP32
- Arduino framework
- LoRa library
- USB serial output

### Ground station

- Python
- Flask
- PySerial
- HTML
- CSS
- JavaScript

---

## 12. Current Development Status

| Subsystem | Status |
|---|---|
| Arduino UNO Q | Working |
| BME280 | Working |
| MQ-2 digital output | Working/integrated |
| RA-02 LoRa transmitter | Working |
| RA-02 LoRa receiver | Working |
| ESP32 receiver | Working |
| Laptop telemetry dashboard | Working |
| USB camera | Working |
| Camera streaming over Wi-Fi | Working |
| YDLIDAR X2 rotation | Working |
| YDLIDAR USB detection | Works directly; hub connection requires resolution |
| LiDAR serial data verification | In progress |
| Local LiDAR obstacle detection | Planned |
| Low-resolution LiDAR LoRa mapping | Planned |
| Dashboard LiDAR map | Planned |
| Thermal camera | Not yet available |
| IP65 enclosure | Not yet implemented |
| Motor subsystem | Skipped for current demonstration |

---

## 13. Safety and Power Considerations

The project uses separate power paths for logic and motors.

Current intended arrangement:

```text
2S 18650 battery
       │
       └── Motor power subsystem

USB-C Power Bank
       │
       └── Arduino UNO Q
```

A previously used HW-133AF buck converter experienced an incorrect-input connection and is not considered trusted for powering the UNO Q.

The UNO Q should not be connected directly to the 2S 18650 battery pack.

---

## 14. Development Strategy

The project is being developed incrementally.

### Stage 1 — Hardware verification

Each subsystem is tested independently:

```text
UNO Q
  ↓
BME280
  ↓
MQ-2
  ↓
LoRa
  ↓
Camera
  ↓
LiDAR
```

### Stage 2 — Integration

Environmental telemetry is integrated first because it is safety-critical.

Camera streaming is then integrated over Wi-Fi.

LiDAR is integrated separately so that local obstacle detection can operate independently from communication bandwidth.

### Stage 3 — Ground station

The Flask dashboard combines:

```text
Camera
+
Environmental telemetry
+
LoRa link status
+
LiDAR visualization
```

---

## 15. Project Design Principle

The central design principle is **communication diversity with safety prioritization**.

The rover does not depend on a single communication path for every type of information.

```text
High-bandwidth perception
        │
        ├── Camera ── Wi-Fi
        │
        └── LiDAR ── Local processing
                       │
                       └── Low-rate map ── LoRa

Safety-critical telemetry
        │
        └── BME280 + MQ-2 ── LoRa
```

This allows the rover to continue performing local obstacle detection while preserving a dedicated communication path for critical environmental information.

---

## 16. Future Expansion

Planned additions include:

- Thermal camera
- Improved LiDAR mapping
- 360°/expanded spatial mapping where appropriate
- IP65-rated enclosure
- More advanced gas sensing
- Improved obstacle classification
- Enhanced ground-station visualization
- Additional rescue-oriented telemetry
- More robust field power management

---

## 17. Repository Structure

A suggested repository structure is:

```text
Mine-Rescue-Rover/
│
├── README.md
│
├── rover/
│   ├── sensors/
│   │   ├── bme280/
│   │   └── mq2/
│   │
│   ├── lora/
│   │   └── transmitter/
│   │
│   ├── lidar/
│   │   └── processing/
│   │
│   └── camera/
│       └── server/
│
├── receiver/
│   └── esp32/
│       └── lora_receiver/
│
├── dashboard/
│   ├── app.py
│   └── templates/
│       └── index.html
│
└── docs/
    ├── architecture/
    ├── wiring/
    └── testing/
```

---

## 18. Quick Start

### Start the UNO Q

Verify the rover-side sensors and LoRa transmitter.

### Start the ESP32 receiver

Connect the ESP32 to the laptop over USB.

Verify that JSON telemetry is being received.

### Start the dashboard

From the dashboard directory:

```bash
python3 app.py
```

Open:

```text
http://127.0.0.1:5001
```

### Verify camera

The UNO Q camera stream should be available at:

```text
http://10.161.47.161:5001/video_feed
```

### Verify LiDAR

Once the USB connection is stable:

```bash
ls /dev/ttyUSB*
```

Expected:

```text
/dev/ttyUSB0
```

The LiDAR integration should then be tested before enabling LiDAR mapping over LoRa.

---

## 19. Disclaimer

This prototype is intended for **research, demonstration, and Smart India Hackathon development purposes**. It should not be treated as a certified mine-safety or life-critical system without appropriate industrial testing, hazardous-environment certification, redundant sensing, validated gas detection, enclosure protection, electromagnetic compatibility testing, and formal safety engineering.

---

## 20. Project Summary

**Mine Rescue Rover** is a multi-modal underground monitoring and rescue platform combining:

- Arduino UNO Q edge computing
- Environmental sensing
- Long-range LoRa communication
- USB optical vision
- LiDAR-based local obstacle detection
- Low-bandwidth remote mapping
- ESP32 communication gateway
- Flask-based operator dashboard

The architecture prioritizes **environmental safety telemetry**, keeps **obstacle detection local**, and uses **Wi-Fi for high-bandwidth camera data** while reserving **LoRa for critical and low-bandwidth information**.
