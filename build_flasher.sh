#!/bin/bash
# Build script for 24Clocks flasher tool
# Compiles Slave firmware, converts to header, copies to flasher, compiles and uploads flasher

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# --- Configuration ---
SERIAL_PORT="/dev/tty.usbserial-1310"
BAUD_RATE="115200"

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
SLAVE_DIR="$PROJECT_ROOT/Slave"
FLASHER_DIR="$PROJECT_ROOT/Flasher"
TOOLS_DIR="$PROJECT_ROOT/tools"

echo -e "${YELLOW}=== 24Clocks Flasher Build Script ===${NC}\n"

# Step 1: Compile Slave
echo -e "${YELLOW}[1/4] Compiling Slave firmware for ATmega2560...${NC}"
cd "$SLAVE_DIR"
arduino-cli compile \
    --fqbn arduino:avr:mega:cpu=atmega2560 \
    --output-dir build \
    Slave.ino

if [ ! -f "build/Slave.ino.hex" ]; then
    echo -e "${RED}ERROR: Slave.ino.hex not found after compilation!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Slave compiled successfully${NC}\n"

# Step 2: Convert HEX to header
echo -e "${YELLOW}[2/4] Converting Slave.ino.hex to firmware_slave.h...${NC}"
python3 "$TOOLS_DIR/hex_to_firmware_header.py" \
    "$SLAVE_DIR/build/Slave.ino.hex" \
    "$FLASHER_DIR/firmware_slave.h" \
    --symbol firmware_slave_hex

if [ ! -f "$FLASHER_DIR/firmware_slave.h" ]; then
    echo -e "${RED}ERROR: firmware_slave.h not created!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Header file generated and copied to flasher${NC}\n"

# Step 3: Compile Flasher
echo -e "${YELLOW}[3/4] Compiling Flasher for ESP32...${NC}"
cd "$FLASHER_DIR"
arduino-cli compile \
    --fqbn esp32:esp32:esp32:JTAGAdapter=default,PSRAM=disabled,PartitionScheme=default,CPUFreq=240,FlashMode=qio,FlashFreq=80,FlashSize=4M,UploadSpeed=460800,LoopCore=1,EventsCore=1,DebugLevel=none,EraseFlash=none,ZigbeeMode=default \
    --output-dir build \
    Flasher.ino

echo -e "${GREEN}✓ Flasher compiled successfully${NC}\n"

# Step 4: Upload to ESP32
echo -e "${YELLOW}[4/4] Uploading Flasher firmware to ESP32...${NC}"
arduino-cli upload \
    --fqbn esp32:esp32:esp32:UploadSpeed=460800 \
    --port "$SERIAL_PORT" \
    --input-dir build \
    Flasher.ino

echo -e "${GREEN}✓ Flasher uploaded successfully${NC}\n"

# Summary
echo -e "${GREEN}=== Build and upload completed successfully! ===${NC}"
echo -e "Slave firmware: ${SLAVE_DIR}/build/Slave.ino.hex"
echo -e "Flasher firmware: ${FLASHER_DIR}/build/Flasher.ino.bin"
echo -e "Header file: ${FLASHER_DIR}/firmware_slave.h"
echo -e "Uploaded to: $SERIAL_PORT"

# Step 5: Open Serial Monitor
echo -e "\n${YELLOW}[5/5] Opening serial monitor...${NC}"
echo -e "${YELLOW}Press CTRL+C to exit monitor${NC}\n"

# Use arduino-cli monitor for better formatting
arduino-cli monitor --port "$SERIAL_PORT" --config baudrate="$BAUD_RATE"
