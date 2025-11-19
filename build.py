#!/usr/bin/env python3
import os
import sys
import time
import subprocess
import argparse
import socket
import re

# --- Configuration ---
SERIAL_PORT = "10.10.0.7"
# SERIAL_PORT = "/dev/tty.usbserial-1310"
OTA_PASSWORD = ""
BAUD_RATE = "115200"

# Paths
PROJECT_ROOT = os.path.dirname(os.path.abspath(__file__))
SLAVE_DIR = os.path.join(PROJECT_ROOT, "Slave")
FLASHER_DIR = os.path.join(PROJECT_ROOT, "Flasher")
MASTER_DIR = os.path.join(PROJECT_ROOT, "Master")
TOOLS_DIR = os.path.join(PROJECT_ROOT, "tools")

# ESP32 FQBN
ESP32_FQBN = "esp32:esp32:esp32:JTAGAdapter=default,PSRAM=disabled,PartitionScheme=default,CPUFreq=240,FlashMode=qio,FlashFreq=80,FlashSize=4M,UploadSpeed=460800,LoopCore=1,EventsCore=1,DebugLevel=none,EraseFlash=none,ZigbeeMode=default"

# Colors
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    NC = '\033[0m'

def run_command(command, cwd=None, shell=False):
    try:
        subprocess.run(command, cwd=cwd, check=True, shell=shell)
    except subprocess.CalledProcessError as e:
        print(f"{Colors.RED}Error executing command: {e}{Colors.NC}")
        sys.exit(1)

def build_flasher():
    print(f"{Colors.YELLOW}=== Building 24Clocks Flasher ==={Colors.NC}\n")

    # Step 1: Compile Slave
    print(f"{Colors.YELLOW}[1/4] Compiling Slave firmware for ATmega2560...{Colors.NC}")
    run_command([
        "arduino-cli", "compile",
        "--fqbn", "arduino:avr:mega:cpu=atmega2560",
        "--output-dir", "build",
        "Slave.ino"
    ], cwd=SLAVE_DIR)

    if not os.path.exists(os.path.join(SLAVE_DIR, "build", "Slave.ino.hex")):
        print(f"{Colors.RED}ERROR: Slave.ino.hex not found after compilation!{Colors.NC}")
        sys.exit(1)
    print(f"{Colors.GREEN}✓ Slave compiled successfully{Colors.NC}\n")

    # Step 2: Convert HEX to header
    print(f"{Colors.YELLOW}[2/4] Converting Slave.ino.hex to firmware_slave.h...{Colors.NC}")
    run_command([
        "python3", os.path.join(TOOLS_DIR, "hex_to_firmware_header.py"),
        os.path.join(SLAVE_DIR, "build", "Slave.ino.hex"),
        os.path.join(FLASHER_DIR, "firmware_slave.h"),
        "--symbol", "firmware_slave_hex"
    ])

    if not os.path.exists(os.path.join(FLASHER_DIR, "firmware_slave.h")):
        print(f"{Colors.RED}ERROR: firmware_slave.h not created!{Colors.NC}")
        sys.exit(1)
    print(f"{Colors.GREEN}✓ Header file generated and copied to flasher{Colors.NC}\n")

    # Step 3: Compile Flasher
    print(f"{Colors.YELLOW}[3/4] Compiling Flasher for ESP32...{Colors.NC}")
    run_command([
        "arduino-cli", "compile",
        "--fqbn", ESP32_FQBN,
        "--output-dir", "build",
        "Flasher.ino"
    ], cwd=FLASHER_DIR)
    print(f"{Colors.GREEN}✓ Flasher compiled successfully{Colors.NC}\n")

    # Step 4: Upload to ESP32
    print(f"{Colors.YELLOW}[4/4] Uploading Flasher firmware to ESP32...{Colors.NC}")
    run_command([
        "arduino-cli", "upload",
        "--fqbn", "esp32:esp32:esp32:UploadSpeed=460800",
        "--port", SERIAL_PORT,
        "--input-dir", "build",
        "--upload-field", f"password={OTA_PASSWORD}",
        "Flasher.ino"
    ], cwd=FLASHER_DIR)
    print(f"{Colors.GREEN}✓ Flasher uploaded successfully{Colors.NC}\n")

    monitor_flasher()

def monitor_flasher():
    is_ip = re.match(r"^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$", SERIAL_PORT)
    
    if is_ip:
        print(f"{Colors.YELLOW}OTA Upload detected (IP address).{Colors.NC}")
        print(f"{Colors.YELLOW}Waiting for device to reboot and connect to WiFi...{Colors.NC}")
        
        max_retries = 30
        count = 0
        connected = False
        while count < max_retries:
            try:
                with socket.create_connection((SERIAL_PORT, 23), timeout=1):
                    connected = True
                    break
            except (socket.timeout, ConnectionRefusedError, OSError):
                pass
            
            print(".", end="", flush=True)
            time.sleep(1)
            count += 1
        print("")

        if not connected:
            print(f"{Colors.RED}Timeout waiting for device. Trying to connect anyway...{Colors.NC}")

        print(f"{Colors.YELLOW}Connecting to Telnet logs... (Waiting for completion message){Colors.NC}")
        
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((SERIAL_PORT, 23))
            s.settimeout(None)
            
            buffer = b""
            success_target = b"Both Slaves Flashed Successfully"
            failure_target = b"Some Slaves Failed to Flash"
            
            while True:
                data = s.recv(1)
                if not data:
                    break
                sys.stdout.buffer.write(data)
                sys.stdout.flush()
                buffer += data
                
                if success_target in buffer:
                    print(f"\n{Colors.GREEN}Flasher job completed successfully.{Colors.NC}")
                    break
                if failure_target in buffer:
                    print(f"\n{Colors.RED}Flasher job FAILED: One or more slaves failed to flash.{Colors.NC}")
                    sys.exit(1)
                    
                if len(buffer) > 4096:
                    buffer = buffer[-2048:]
            s.close()
        except Exception as e:
            print(f"\nConnection failed: {e}")
            sys.exit(1)

    else:
        print(f"\n{Colors.YELLOW}[5/5] Opening serial monitor...{Colors.NC}")
        print(f"{Colors.YELLOW}Waiting for completion message...{Colors.NC}\n")
        
        cmd = ["arduino-cli", "monitor", "--port", SERIAL_PORT, "--config", f"baudrate={BAUD_RATE}"]
        success_target = b"Both Slaves Flashed Successfully"
        failure_target = b"Some Slaves Failed to Flash"
        
        try:
            process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            buffer = b""
            while True:
                byte = process.stdout.read(1)
                if not byte:
                    break
                sys.stdout.buffer.write(byte)
                sys.stdout.flush()
                buffer += byte
                
                if success_target in buffer:
                    process.terminate()
                    print(f"\n{Colors.GREEN}Flasher job completed successfully.{Colors.NC}")
                    break
                if failure_target in buffer:
                    process.terminate()
                    print(f"\n{Colors.RED}Flasher job FAILED: One or more slaves failed to flash.{Colors.NC}")
                    sys.exit(1)
                    
                if len(buffer) > 4096:
                    buffer = buffer[-2048:]
        except Exception as e:
            print(f"\nError: {e}")
            sys.exit(1)

def build_master():
    print(f"{Colors.YELLOW}=== Building 24Clocks Master ==={Colors.NC}\n")

    # Step 1: Compile Master
    print(f"{Colors.YELLOW}[1/2] Compiling Master for ESP32...{Colors.NC}")
    run_command([
        "arduino-cli", "compile",
        "--fqbn", ESP32_FQBN,
        "--output-dir", "build",
        "Master.ino"
    ], cwd=MASTER_DIR)
    print(f"{Colors.GREEN}✓ Master compiled successfully{Colors.NC}\n")

    # Step 2: Upload to ESP32
    print(f"{Colors.YELLOW}[2/2] Uploading Master firmware to ESP32...{Colors.NC}")
    run_command([
        "arduino-cli", "upload",
        "--fqbn", "esp32:esp32:esp32:UploadSpeed=460800",
        "--port", SERIAL_PORT,
        "--input-dir", "build",
        "--upload-field", f"password={OTA_PASSWORD}",
        "Master.ino"
    ], cwd=MASTER_DIR)
    print(f"{Colors.GREEN}✓ Master uploaded successfully{Colors.NC}\n")

    monitor_master()

def monitor_master():
    is_ip = re.match(r"^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$", SERIAL_PORT)
    if is_ip:
        print(f"{Colors.YELLOW}OTA Upload detected (IP address).{Colors.NC}")
        print(f"{Colors.YELLOW}Note: Master does not support Telnet logging. Connect via USB to view Serial logs.{Colors.NC}")
    else:
        print(f"\n{Colors.YELLOW}[3/3] Opening serial monitor...{Colors.NC}")
        print(f"{Colors.YELLOW}Press CTRL+C to exit monitor{Colors.NC}\n")
        try:
            subprocess.run(["arduino-cli", "monitor", "--port", SERIAL_PORT, "--config", f"baudrate={BAUD_RATE}"])
        except KeyboardInterrupt:
            pass

def main():
    parser = argparse.ArgumentParser(description="Unified build script for 24Clocks")
    parser.add_argument("target", choices=["flasher", "master", "full"], help="Target to build")
    args = parser.parse_args()

    if args.target == "flasher":
        build_flasher()
    elif args.target == "master":
        build_master()
    elif args.target == "full":
        build_flasher()
        print(f"{Colors.YELLOW}Waiting 2 seconds before starting Master build...{Colors.NC}")
        time.sleep(2)
        build_master()

if __name__ == "__main__":
    main()
