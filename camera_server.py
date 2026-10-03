from flask import Flask, Response
import cv2
import time

app = Flask(__name__)

CAMERA_DEVICE = "/dev/video1"

camera = cv2.VideoCapture(CAMERA_DEVICE, cv2.CAP_V4L2)
# Request a reasonable resolution for Wi-Fi streaming
camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
camera.set(cv2.CAP_PROP_FPS, 15)

if not camera.isOpened():
    print("ERROR: Could not open camera:", CAMERA_DEVICE)
    exit(1)

print("Camera opened:", CAMERA_DEVICE)


def generate_frames():
    while True:
        success, frame = camera.read()

        if not success:
            print("Camera frame read failed")
            time.sleep(0.1)
            continue

        # JPEG compression
        ret, buffer = cv2.imencode(
            ".jpg",
            frame,
            [cv2.IMWRITE_JPEG_QUALITY, 70]
        )

        if not ret:
            continue

        frame_bytes = buffer.tobytes()

        yield (
            b"--frame\r\n"
            b"Content-Type: image/jpeg\r\n\r\n"
            + frame_bytes
            + b"\r\n"
        )


@app.route("/")
def index():
    return """
    <html>
    <head>
        <title>Mine Rescue Rover Camera</title>
    </head>
    <body>
        <h1>UNO Q Camera</h1>
        <img src="/video_feed" width="640">
    </body>
    </html>
    """


@app.route("/video_feed")
def video_feed():
    return Response(
        generate_frames(),
        mimetype="multipart/x-mixed-replace; boundary=frame"
    )


if __name__ == "__main__":
    print("Starting camera server...")
    print("Camera stream: http://<UNO-Q-IP>:5001/video_feed")

    app.run(
        host="0.0.0.0",
        port=5001,
        threaded=True
    )

