# 24Clocks

This repository contains the source code (Master, Slave, and Flasher) for a 24-dial kinetic clock.

For the complete project description, construction details, hardware, and PCB files, please refer to the project on Hackaday.io: [[HACKADAY.IO PROJECT](https://hackaday.io/project/204370-clock)]

## Code Architecture (Master/Slave/Flasher)

The system consists of three main software components:

*   **Master (ESP32):** The code in `Master/` runs on an ESP32. It manages network connectivity, time synchronization, and command logic for controlling the Slave boards.
*   **Slave (x2) (Arduino Mega):** The code in `Slave/` runs on two Arduino Mega 2560 boards. This code receives commands and handles low-level motor control.
*   **Flasher (ESP32):** The code in `Flasher/` is a standalone tool that runs on an ESP32 and is used to flash new firmware onto the Slave boards via serial communication using the STK500 protocol.

### Master (ESP32)

*   **WiFi and Time Synchronization:** Connects to a WiFi network (credentials in `Master/wifiManager.cpp`) and uses the `ezTime` library to get the current time via NTP.
*   **TCP Server (Telnet):** The `wifiManager.cpp` file starts a TCP server on port 80. This acts as a command-line interface to receive commands (e.g., `SETTIME=HHMM`, `SETHOME`) from a Telnet client or the 24Client Android app.
*   **Command Logic:** The main loop (`Master.ino`) checks the time. When the minute changes, it formats a command string (e.g., `CMDSETTIME=1209;`) and sends it to both Slaves via the `serialLink` library.

### Flasher (ESP32)

*   **AVR Programming Tool:** The Flasher is a standalone utility that programs the ATmega2560 Slave boards using the STK500 protocol over serial communication.
*   **Flashing Process:** The process starts automatically as soon as a serial monitor is connected to the ESP32. The flasher waits for the serial connection to ensure that the flashing progress and results are visible. Once connected, it performs a specific reset sequence on the ATmega2560 to enter the bootloader, sends the firmware embedded in `firmware_slave.h` page by page, and verifies the written data.
*   **STK500 Protocol Implementation:** The core logic in `avr_flash_arduino.cpp` is based on code from [[Laukik Hase's OTA_update_AVR_using_ESP32 project](https://github.com/ESP32-Musings/OTA_update_AVR_using_ESP32)], adapted for the Arduino framework.

### Slave (Arduino Mega 2560)

*   **Division of Tasks:** The system is divided vertically. The `slaveOffset` variable defines which side a Slave controls (0 for Slave 1/Left, 1 for Slave 2/Right). This value is set dynamically by the Master at runtime via a `SETSLAVEOFFSET` command, allowing the same firmware to be used on both boards.
    *   **Slave 1 (Left):** Manages the H1 and M1 digits.
    *   **Slave 2 (Right):** Manages the H2 and M2 digits.
*   **Command Reception:** Listens on the `Serial` port for commands sent from the Master.
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
*   **`avr_flash_arduino`**: (`Flasher/avr_flash_arduino.*`) A library implementing the STK500 protocol to flash ATmega2560 boards. Based on [Laukik Hase's work](https://github.com/ESP32-Musings/OTA_update_AVR_using_ESP32).

## Setup and Usage

1.  **Software Configuration:**
    *   Open the `Master/Master.ino` and `Slave/Slave.ino` sketches in the Arduino IDE.
    *   Install the required external libraries (e.g., `ezTime`) using the Library Manager.
    *   In `Master/wifiManager.cpp`, configure your WiFi credentials (`ssid` and `password`).
2.  **Upload Code to Slaves:**
    *   **Method 1 (Direct Upload):** Upload `Slave.ino` directly to both Arduino Mega boards using a USB cable.
    *   **Method 2 (Using the Serial Flasher):** This method uses a dedicated ESP32 to program the Slave boards without needing to connect them to a computer.
        *   Compile the `Slave.ino` sketch to generate `Slave/build/Slave.ino.hex`.
        *   Run `tools/hex_to_firmware_header.py` to convert the `.hex` file into `firmware_slave.h`, which embeds the firmware into the Flasher code.
        *   Upload `Flasher.ino` to the ESP32 board that will serve as the programmer.
        *   Connect the Flasher ESP32 to the target Slave board via serial pins (TX->RX, RX->TX) and a reset control pin.
        *   Open the serial monitor for the Flasher. The flashing process will start automatically.
3.  **Upload Master:**
    *   Upload `Master.ino` to the ESP32 board.
4.  **Homing Prerequisite:**
    *   This code operates "open-loop", meaning it assumes the initial position of the hands and cannot verify it.
    *   Before starting the system, it is mandatory to manually position all 48 hands to the "home" position (vertical, 12 o'clock). The `SETHOME` command in the software will move the hands to 0°, which must correspond to this physical position.
4.  **Operation:**
    *   On startup, the Master will connect to WiFi, synchronize the time, and start sending commands to the Slaves.
    *   You can connect to the Master's IP address via a Telnet client (port 80) to send commands manually.

### Build Automation (`build_flasher.sh`)

The repository includes the `build_flasher.sh` script, to automate the process of preparing and uploading the `Flasher` firmware using `arduino-cli`.

**What it does:**
1.  **Compiles the Slave Firmware:** It builds the `Slave.ino` sketch for the ATmega2560.
2.  **Generates Firmware Header:** It runs `tools/hex_to_firmware_header.py` to convert the compiled `Slave.ino.hex` into a C header file (`firmware_slave.h`).
3.  **Compiles the Flasher:** It builds the `Flasher.ino` sketch, embedding the new Slave firmware.
4.  **Uploads to ESP32:** It uploads the compiled Flasher firmware to the ESP32.
5.  **Opens Serial Monitor:** It automatically opens a serial monitor to show the flashing progress.

**Note:** You nay need to edit the `SERIAL_PORT` and `BAUD_RATE` variables in the script to match your configuration.
