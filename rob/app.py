from flask import Flask, jsonify, render_template
import serial
import threading
import json
import time
import glob

app = Flask(__name__)

# =====================================================
# CONFIGURATION
# =====================================================

BAUD_RATE = 115200

SERIAL_PORT = None

# =====================================================
# UNO Q CAMERA
# =====================================================

UNO_Q_IP = "10.161.47.161"
CAMERA_PORT = 5001

CAMERA_URL = f"http://{UNO_Q_IP}:{CAMERA_PORT}/video_feed"


# =====================================================
# SHARED TELEMETRY
# =====================================================

telemetry = {
    "temp": None,
    "humidity": None,
    "pressure": None,
    "gas": "NO DATA",
    "rssi": None,
    "snr": None,
    "last_update": None,
    "lora_connected": False
}

lock = threading.Lock()


# =====================================================
# FIND ESP32 SERIAL PORT
# =====================================================

def find_serial_port():

    ports = []

    ports.extend(glob.glob("/dev/ttyUSB*"))
    ports.extend(glob.glob("/dev/ttyACM*"))

    if ports:

        print("Possible ESP32 ports:")

        for port in ports:
            print("  ", port)

        return ports[0]

    return None


# =====================================================
# SERIAL READER
# =====================================================

def serial_reader():

    while True:

        port = SERIAL_PORT or find_serial_port()

        if port is None:

            print("ESP32 serial port not found.")

            with lock:
                telemetry["lora_connected"] = False

            time.sleep(3)

            continue

        ser = None

        try:

            print()
            print("Opening ESP32 serial port:")
            print(port)

            ser = serial.Serial(
                port,
                BAUD_RATE,
                timeout=1
            )

            print("ESP32 connected.")

            while True:

                line = ser.readline().decode(
                    "utf-8",
                    errors="ignore"
                ).strip()

                if not line:
                    continue

                # =================================================
                # ONLY PROCESS JSON TELEMETRY
                # =================================================

                if not line.startswith("{"):
                    continue

                try:

                    data = json.loads(line)

                    with lock:

                        telemetry["temp"] = data.get("temp")

                        telemetry["humidity"] = data.get(
                            "humidity"
                        )

                        telemetry["pressure"] = data.get(
                            "pressure"
                        )

                        telemetry["gas"] = data.get(
                            "gas",
                            "UNKNOWN"
                        )

                        telemetry["rssi"] = data.get(
                            "rssi"
                        )

                        telemetry["snr"] = data.get(
                            "snr"
                        )

                        telemetry["last_update"] = time.time()

                        telemetry["lora_connected"] = True

                    print("Telemetry:", data)

                except json.JSONDecodeError:

                    print(
                        "Invalid JSON received:",
                        line
                    )

        except Exception as e:

            print(
                "Serial error:",
                e
            )

            with lock:
                telemetry["lora_connected"] = False

            time.sleep(3)

        finally:

            if ser is not None:

                try:
                    ser.close()

                except:
                    pass


# =====================================================
# MAIN DASHBOARD
# =====================================================

@app.route("/")
def dashboard():

    return render_template(
        "index.html",
        camera_url=CAMERA_URL
    )


# =====================================================
# TELEMETRY API
# =====================================================

@app.route("/api/data")
def api_data():

    with lock:

        data = dict(telemetry)

    return jsonify(data)


# =====================================================
# MAIN
# =====================================================

if __name__ == "__main__":

    # Start ESP32 serial reader
    thread = threading.Thread(
        target=serial_reader,
        daemon=True
    )

    thread.start()

    print()
    print("==============================================")
    print("       MINE RESCUE ROVER DASHBOARD")
    print("==============================================")
    print()

    print("Dashboard:")
    print("http://127.0.0.1:5001")

    print()

    print("UNO Q Camera:")
    print(CAMERA_URL)

    print()

    print("ESP32 telemetry:")
    print("Automatic serial-port detection")

    print()

    app.run(
        host="0.0.0.0",
        port=5001,
        debug=False,
        threaded=True
    )
