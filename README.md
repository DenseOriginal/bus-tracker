# Midttrafik Live Tracker

A custom hardware integration for tracking Midttrafik buses in real-time. This project consists of a Deno-based backend that processes bus data and an ESP8266-based hardware client that displays departure times using RGB LEDs.

![Showcase](docs/showcase.png)

## Features

* **Real-time Tracking**: Fetches live departure times for specific stops.
* **Smart Walking Time**: Calculates exactly when you need to leave based on configured walking times to the stops.
* **Hardware Display**: Visual feedback using RGB LEDs on an ESP8266.
* **Departure Alarm**: Notify when it's time to leave.

## Structure

* **`/arduino-live-tracker`**: Contains the C++ code for the ESP8266.
* **`/src`**: TypeScript source code for the backend (API wrappers, types, utilities).

## Getting Started

### Backend (Deno)

1. **Install Deno**: Ensure you have Deno installed on your system.
2. **Environment Setup**:
    Create a `.env` file in the root directory based on `.env.example`. You will need:

    ```env
    MIDTTRAFIK_API_BASE_URL=your_midttrafik_api_url
    ```

3. **Run the Server**:
    Start the backend server which processes the API data and serves it to the hardware client.

    ```bash
    deno run -A server.ts
    ```

    The server listens on port `4242`.

    Alternatively, run the CLI tool to see departures in the terminal:

    ```bash
    deno run -A main.ts
    ```

### Hardware (ESP8266)

The hardware client is built for an ESP8266 microcontroller.

**Prerequisites:**

* ESP8266 Board (e.g., NodeMCU, Wemos D1 Mini)
* 2x RGB LEDs (connected to `D3`)
* Button (connected to `D2`)
* Buzzer (connected to `D1`)
* Arduino IDE with ESP8266 board support.

**Setup:**

1. Navigate to `arduino-live-tracker/`.
2. Copy `conf.h.example` to `conf.h`.
3. Fill in your configuration:

    ```cpp
    #define BUS_API_URL "http://<your-server-ip>:4242/"
    #define WIFI_SSID "your_wifi_ssid"
    #define WIFI_PASSWORD "your_wifi_password"
    ```

4. Open `arduino-live-tracker.ino` in Arduino IDE.
5. Install required libraries via Library Manager:
    * `ArduinoJson`
    * `Adafruit NeoPixel`
    * `PinButton`
6. Upload the sketch to your board.

## Logic

The device stays awake for a configured time (default 5 minutes) after a button press, fetching data every 30 seconds. The RGB LEDs indicate the departure status for the monitored stops, helping you decide when to start walking.
