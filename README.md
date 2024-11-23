# ESP32-CAM Web-Controlled Rover

A web-controlled rover built with ESP32-CAM that provides real-time video streaming and keyboard-based movement control through a web interface.

## Features

- Real-time video streaming from ESP32-CAM
- Web-based control interface
- Keyboard controls for movement (W, A, S, D)
- Motor control for forward, backward, left, and right movements
- Easy-to-use web interface
- Access Point (AP) mode for direct connection

## Hardware Requirements

- ESP32-CAM module
- DC Motors (2x) for movement
- Motor Driver (L298N or similar)
- Power supply (recommended 7.4V for motors)
- Chassis for the rover
- Wheels
- Battery holder
- Jumper wires

## Pin Configuration

### Motor Connections
- Motor A (Forward/Backward):
  - Forward: GPIO 12
  - Backward: GPIO 13
- Motor B (Left/Right):
  - Left: GPIO 14
  - Right: GPIO 15

### Camera Pins
The ESP32-CAM module uses the following pins for the camera interface:
- XCLK: GPIO 0
- PCLK: GPIO 22
- VSYNC: GPIO 25
- HREF: GPIO 23
- SDA: GPIO 26
- SCL: GPIO 27
- D0-D7: GPIO 5, 18, 19, 21, 36, 39, 34, 35

## Software Setup

1. Install the required libraries in Arduino IDE:
   - WebSocketsServer
   - ESP32 Board Support Package

2. Upload the code to your ESP32-CAM module

## Usage

1. Power up the ESP32-CAM rover
2. Connect to the WiFi network:
   - SSID: `ESP32-CAM-AP`
   - Password: `12345678`
3. Open a web browser and navigate to: `http://192.168.1.1`
4. Click "Start Stream" to begin video streaming
5. Use keyboard controls to move the rover:
   - W: Move forward
   - S: Move backward
   - A: Turn left
   - D: Turn right
   - E: Stop movement

## Control Interface

The web interface provides:
- Live video stream from the ESP32-CAM
- Stream control buttons (Start/Stop)
- Movement control using keyboard keys
- Visual feedback of the rover's status

## Network Configuration

The rover creates its own WiFi network:
- Mode: Access Point (AP)
- IP Address: 192.168.1.1
- Subnet Mask: 255.255.255.0
- WebSocket Port: 81

## Running

Compilation:
```
arduino-cli compile --fqbn esp32:esp32:esp32cam ./camera_ws.ino
```

Flashing:
```
arduino-cli upload --port COM3 --fqbn esp32:esp32:esp32cam ./camera_ws.ino
```


## Troubleshooting

1. If the video stream doesn't start:
   - Check if the camera is properly connected
   - Try refreshing the web page
   - Ensure you're connected to the correct WiFi network

2. If motors don't respond:
   - Verify motor connections
   - Check motor driver power supply
   - Ensure proper GPIO connections

3. If unable to connect to WiFi:
   - Reset the ESP32-CAM
   - Verify you're trying to connect to the correct network
   - Check if the device is powered properly

## Contributing

Feel free to submit issues and enhancement requests!

## License

This project is open-source and available under the MIT License.

## Credits

Built using ESP32-CAM and Arduino framework. Special thanks to the ESP32 community for their valuable resources and support.


