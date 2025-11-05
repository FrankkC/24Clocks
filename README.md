# 24Clocks

This repository contains the source code (Master and Slave) for a 24-dial kinetic clock.

For the complete project description, construction details, hardware, and PCB files, please refer to the project on Hackaday.io: [[HACKADAY.IO PROJECT](https://hackaday.io/project/204370-clock)]

## Code Architecture (Master/Slave)

The system consists of three main software components:

*   **Master (ESP32):** The code in `Master/` runs on an ESP32. It manages network connectivity, time synchronization, command logic, and can flash the Slave boards with new firmware.
*   **Slave (x2) (Arduino Mega):** The code in `Slave/` runs on two Arduino Mega 2560 boards. This code receives commands and handles low-level motor control.
*   **Firmware:** The `Slave/build/Slave.ino.hex` file is the compiled firmware for the Slaves, which the Master can deploy.

### Master (ESP32)

*   **WiFi and Time Synchronization:** Connects to a WiFi network (credentials in `Master/wifiManager.cpp`) and uses the `ezTime` library to get the current time via NTP.
*   **TCP Server (Telnet):** The `wifiManager.cpp` file starts a TCP server on port 80. This acts as a command-line interface to receive commands (e.g., `SETTIME=HHMM`, `SETHOME`, `FLASH`) from a Telnet client or the 24Client Android app.
*   **Command Logic:** The main loop (`Master.ino`) checks the time. When the minute changes, it formats a command string (e.g., `CMDSETTIME=1209;`) and sends it to both Slaves via the `serialLink` library.
*   **AVR Flasher:** The `AVRFlasher` component allows the ESP32 to flash new firmware onto the Slave boards directly via serial communication. This is triggered by the `FLASH` command, which uses the firmware stored in `firmware_slave.h`.

### Slave (Arduino Mega 2560)

*   **Division of Tasks:** The system is divided vertically. The `slaveOffset` variable defines which side a Slave controls (0 for Left, 1 for Right). This value is set dynamically by the Master at runtime via a `SETSLAVEOFFSET` command, allowing the same firmware to be used on both boards.
    *   **Slave 0 (Left):** Manages the H1 and M1 digits.
    *   **Slave 1 (Right):** Manages the H2 and M2 digits.
*   **Command Reception:** Listens on the `Serial3` port for commands sent from the Master.
*   **Motor Control:** Uses the `SwitecX12` library to drive the stepper motors. The current version moves the motors at a constant speed. For the very outdated version with acceleration support, please refer to the tag `last-commit-with-accel-support`.
*   **Font Matrix (ClockPositions.h):** The heart of the visual logic. It contains an array that maps each digit (0-9) to a set of angles (0, 90, 180, 270) for the 12 hands that make up that specific digit.
*   **Pin Mapping (ClockPins.h):** Defines the GPIO pins of the Arduino Mega used to drive the motor drivers.

## Libraries and Dependencies

The project relies on a few external libraries and some internal ones located in the `lib/` directory.

### External Libraries (managed via Arduino IDE)
*   **`ezTime`**: Used by the Master for NTP time synchronization.

### Internal Libraries (included in this repository)
*   **`serialLink`**: (`lib/serialLink/`) A custom library for handling serial communication between the Master and Slaves.
*   **`SwitecX12`**: (`Slave/SwitecX12.*`) A library for controlling the X12 stepper motors. Note: This is included directly in the Slave sketch folder. The library is based on the original work by Guy Carpenter (Clearwater Software, 2017) and has been modified for the specific needs of this project.

### Included Components (not technically libraries)
*   **`wifiManager`**: (`Master/wifiManager.*`) A component for managing WiFi connection and the Telnet command server on the Master.
*   **`AVRFlasher`**: (`Master/AVRFlasher.*`) A component that enables the Master to flash firmware on the Slaves.

## Setup and Usage

1.  **Software Configuration:**
    *   Open the `Master/Master.ino` and `Slave/Slave.ino` sketches in the Arduino IDE.
    *   Install the required external libraries (e.g., `ezTime`) using the Library Manager.
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
5.  **Updating Slave Firmware via Master:**
    *   To update the Slaves, first compile the `Slave.ino` sketch.
    *   Convert the resulting `.hex` file into a C-style byte array.
    *   Replace the contents of `Master/firmware_slave.h` with this new byte array.
    *   Upload the updated `Master.ino` to the ESP32.
    *   Send the `FLASH` command via Telnet to begin the flashing process for both Slaves.