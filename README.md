# 24Clocks

This repository contains the source code (Master and Slave) for a 24-dial kinetic clock.

For the complete project description, construction details, hardware, and PCB files, please refer to the project on Hackaday.io: [[HACKADAY.IO PROJECT](https://hackaday.io/project/204370-clock)]

## Code Architecture (Master/Slave)

The system consists of three main software components:

*   **Master (ESP32):** The code in `Master/` runs on an ESP32. It manages network connectivity, time synchronization, and command logic.
*   **Slave (x2) (Arduino Mega):** The code in `Slave/` runs on two Arduino Mega 2560 boards. This code receives commands and handles low-level motor control.

### Master (ESP32)

*   **WiFi and Time Synchronization:** Connects to a WiFi network (credentials in `Master/wifiManager.cpp`) and uses the `ezTime` library to get the current time via NTP.
*   **TCP Server (Telnet):** The `wifiManager.cpp` file starts a TCP server on port 80. This acts as a command-line interface to receive commands (e.g., `SETTIME=HHMM`, `SETHOME`) from a Telnet client or the 24Client Android app.
*   **Command Logic:** The main loop (`Master.ino`) checks the time. When the minute changes, it formats a command string (e.g., `CMDSETTIME=1209;`) and sends it to both Slaves via `serialLink.cpp`.

### Slave (Arduino Mega 2560)

*   **Division of Tasks:** The system is divided vertically. The `slaveOffset` variable (set to `0` for one Slave, `1` for the other) defines the side:
    *   **Slave 0 (Left):** Manages the H1 and M1 digits.
    *   **Slave 1 (Right):** Manages the H2 and M2 digits.
*   **Command Reception:** Listens on the `Serial3` port for commands sent from the Master.
*   **Motor Control:** Uses the `SwitecX12` library to drive the stepper motors. The current version moves the motors at a constant speed. For the very outdated version with acceleration support, please refer to the tag `last-commit-with-accel-support`.
*   **Font Matrix (ClockPositions.h):** The heart of the visual logic. It contains an array that maps each digit (0-9) to a set of angles (0, 90, 180, 270) for the 12 hands that make up that specific digit.
*   **Pin Mapping (ClockPins.h):** Defines the GPIO pins of the Arduino Mega used to drive the motor drivers.

## Libraries and Dependencies

*   Arduino IDE
*   **Libraries (Master):**
    *   `ezTime`
    *   `WiFi`
*   **Libraries (Shared):**
    *   `serialLink` (included)
    *   `wifiManager` (included)
    *   `SwitecX12` (included)

## Setup and Usage

1.  **Software Configuration:**
    *   Open the `Master/Master.ino` and `Slave/Slave.ino` sketches.
    *   Install the required libraries.
    *   In `Master/wifiManager.cpp`, configure your WiFi credentials (`ssid` and `password`).
2.  **Upload Code:**
    *   Upload `Master.ino` to the ESP32 board.
    *   Upload `Slave.ino` to the first Arduino Mega (making sure `slaveOffset = 0`).
    *   Upload `Slave.ino` to the second Arduino Mega (setting `slaveOffset = 1`).
3.  **Homing Prerequisite:**
    *   This code operates "open-loop", meaning it assumes the initial position of the hands and cannot verify it.
    *   Before starting the system, it is mandatory to manually position all 48 hands to the "home" position (vertical, 12 o'clock). The `SETHOME` command in the software will move the hands to 0°, which must correspond to this physical position.
4.  **Operation:**
    *   On startup, the Master will connect to WiFi, synchronize the time, and start sending commands to the Slaves.
    *   You can connect to the Master's IP address via a Telnet client (port 80) to send commands manually.
