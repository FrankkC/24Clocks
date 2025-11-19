/*
 * AVR Flash Library for ESP32
 * Implements STK500 protocol to flash ATmega2560
 * 
 * Based on code from Laukik Hase's project:
 * https://github.com/ESP32-Musings/OTA_update_AVR_using_ESP32
 */

#include "avr_flash_arduino.h"
#include "firmware_slave.h"
#include "esp_heap_caps.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Global debugger stream
Print* avr_debugger = &Serial;

// --- CONFIGURATION ---
#define BLOCK_SIZE 256      // AVR flash page size

// Global configuration for current flash operation
static AVRFlashConfig g_current_config;

// --- STK500 PROTOCOL CONSTANTS ---
const uint8_t STK_OK = 0x10;
const uint8_t STK_INSYNC = 0x14;
const uint8_t Cmnd_STK_PROG_PAGE = 0x64;
const uint8_t Cmnd_STK_READ_PAGE = 0x74;
const uint8_t Cmnd_STK_LOAD_ADDRESS = 0x55;
const uint8_t Cmnd_STK_GET_SYNC = 0x30;
const uint8_t Cmnd_STK_LEAVE_PROGMODE = 0x51;
const uint8_t Cmnd_STK_ENTER_PROGMODE = 0x50;
const uint8_t Sync_CRC_EOP = 0x20;
const uint8_t Cmnd_STK_SET_DEVICE = 0x42;
const uint8_t Cmnd_STK_SET_DEVICE_EXT = 0x45;

// Increase timeouts to allow AVR time to complete internal page programming.
// The esp-idf implementation yields/waits in RTOS loops; a larger timeout
// here makes the Arduino port behave more like the esp-idf flow.
const unsigned long SERIAL_TIMEOUT_MS = 2000;
const int MAX_SETUP_ATTEMPTS = 3;

// UART driver configuration matching esp-idf implementation
#define AVR_UART_RX_BUF_SIZE 2048

static void init_avr_uart()
{
    // Uninstall previous UART driver if it exists
    uart_driver_delete(g_current_config.uart_num);

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_param_config(g_current_config.uart_num, &uart_config);
    uart_set_pin(g_current_config.uart_num, g_current_config.tx_pin, g_current_config.rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(g_current_config.uart_num, AVR_UART_RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

static bool avr_expect_in_sync_ok(const char* step, unsigned long timeout_ms = SERIAL_TIMEOUT_MS, bool verbose = false);
static bool avr_exec_param(uint8_t cmd, const uint8_t* params, size_t count, const char* step);
static void avr_reset_sequence();
static bool avr_set_prog_params();
static bool avr_set_ext_prog_params();
static int compute_firmware_size(const char* firmware_hex);
static bool populate_firmware_image(const char* firmware_hex, uint8_t* image, int image_size);
static uint8_t* allocate_firmware_buffer(size_t size);
static void log_verification_mismatch(int absolute_offset, const uint8_t* expected, const uint8_t* actual, size_t length);


// --- UART COMMUNICATION FUNCTIONS ---

static void sendData(const uint8_t* data, size_t size) {
    // Use uart driver write (blocks until queued)
    uart_write_bytes(g_current_config.uart_num, (const char*)data, size);
    // Ensure data is pushed out of UART HAL FIFO
    uart_wait_tx_done(g_current_config.uart_num, pdMS_TO_TICKS(100));
}

static int receiveData(uint8_t* buffer, size_t size, unsigned long timeout_ms) {
    // Use a buffered-wait similar to the esp-idf implementation: poll the
    // UART driver's buffered-data length until we have the expected number of
    // bytes or timeout, then read them in one call. This avoids partial reads
    // and better matches the original timing.
    if (size == 0) return 0;

    unsigned long start = millis();
    size_t available = 0;
    while ((millis() - start) < timeout_ms) {
        uart_get_buffered_data_len(g_current_config.uart_num, &available);
        if (available >= size) break;
        // yield like esp-idf (small delay)
        vTaskDelay(pdMS_TO_TICKS(2));
    }

    if (available == 0) return 0;
    size_t toRead = (available >= size) ? size : available;
    int read = uart_read_bytes(g_current_config.uart_num, (char*)buffer, toRead, pdMS_TO_TICKS(100));
    return read;
}

static void flushSerial() {
    // Drain UART RX buffer
    uart_flush_input(g_current_config.uart_num);
}

static bool avr_expect_in_sync_ok(const char* step, unsigned long timeout_ms, bool verbose) {
    uint8_t resp[2] = {0, 0};
    int received = receiveData(resp, 2, timeout_ms);

    if (received == 2 && resp[0] == STK_INSYNC && resp[1] == STK_OK) {
        if (verbose) {
            avr_debugger->printf("%s: Received %d bytes -> [0x%02X, 0x%02X]\n", step, received, resp[0], resp[1]);
        }
        return true;
    }

    if (received > 0) {
        avr_debugger->printf("%s: Received %d bytes -> [0x%02X, 0x%02X]\n", step, received, resp[0], resp[1]);
    } else {
        avr_debugger->printf("%s: Received 0 bytes (timeout).\n", step);
    }

    return false;
}

static bool avr_exec_param(uint8_t cmd, const uint8_t* params, size_t count, const char* step) {
    flushSerial();

    sendData(&cmd, 1);
    if (count > 0) {
        sendData(params, count);
    }
    uint8_t eop = Sync_CRC_EOP;
    sendData(&eop, 1);

    return avr_expect_in_sync_ok(step, SERIAL_TIMEOUT_MS);
}

static void avr_reset_sequence() {
    digitalWrite(g_current_config.reset_pin, LOW);
    delay(100);
    digitalWrite(g_current_config.reset_pin, HIGH);
    delay(100);
    digitalWrite(g_current_config.reset_pin, LOW);
    delay(100);
    digitalWrite(g_current_config.reset_pin, HIGH);
    delay(100);
}

static bool avr_set_prog_params() {
    const uint8_t params[] = {
        0x86, /* devicecode */
        0x00, /* revision */
        0x00, /* progtype */
        0x01, /* parmode */
        0x01, /* polling */
        0x01, /* selftimed */
        0x01, /* lockbytes */
        0x03, /* fusebytes */
        0xff, /* flashpollval1 */
        0xff, /* flashpollval2 */
        0xff, /* eeprompollval1 */
        0xff, /* eeprompollval2 */
        0x00, /* pagesizehigh */
        0x80, /* pagesizelow */
        0x10, /* eepromsizehigh */
        0x00, /* eepromsizelow */
        0x00, /* flashsize4 */
        0x04, /* flashsize3 */
        0x00, /* flashsize2 */
        0x00  /* flashsize1 */
    };

    avr_debugger->println("Setting programming parameters...");
    return avr_exec_param(Cmnd_STK_SET_DEVICE, params, sizeof(params), "Set prog params");
}

static bool avr_set_ext_prog_params() {
    const uint8_t params[] = {0x05, 0x04, 0xd7, 0xc2, 0x00};
    avr_debugger->println("Setting extended programming parameters...");
    return avr_exec_param(Cmnd_STK_SET_DEVICE_EXT, params, sizeof(params), "Set ext prog params");
}


// --- STK500 PROTOCOL FUNCTIONS (WITH DIAGNOSTICS) ---

static bool avr_get_sync() {
    flushSerial();
    uint8_t cmd[] = {Cmnd_STK_GET_SYNC, Sync_CRC_EOP};
    for (int retries = 0; retries < 5; retries++) {
        sendData(cmd, sizeof(cmd));
        if (avr_expect_in_sync_ok("Sync attempt", 200)) {
            return true;
        }
        delay(20);
    }
    return false;
}

static bool avr_enter_progmode() {
    flushSerial();
    uint8_t cmd[] = {Cmnd_STK_ENTER_PROGMODE, Sync_CRC_EOP};
    sendData(cmd, sizeof(cmd));
    return avr_expect_in_sync_ok("Enter progmode", 200);
}

static bool avr_leave_progmode() {
    flushSerial();
    uint8_t cmd[] = {Cmnd_STK_LEAVE_PROGMODE, Sync_CRC_EOP};
    sendData(cmd, sizeof(cmd));
    return avr_expect_in_sync_ok("Leave progmode", 200);
}

static bool avr_setup_device() {
    // Initialize UART driver (if available) before any UART operations
    init_avr_uart();
    pinMode(g_current_config.reset_pin, OUTPUT);
    digitalWrite(g_current_config.reset_pin, HIGH);

    avr_debugger->println("Resetting AVR and trying to enter programming mode...");
    for (int attempt = 1; attempt <= MAX_SETUP_ATTEMPTS; attempt++) {
        avr_debugger->printf("Setup attempt %d of %d\n", attempt, MAX_SETUP_ATTEMPTS);
        flushSerial();
        avr_reset_sequence();

        if (!avr_get_sync()) {
            avr_debugger->println("Sync failed, retrying setup...");
            continue;
        }

        if (!avr_set_prog_params()) {
            avr_debugger->println("Failed to set programming parameters, retrying setup...");
            continue;
        }

        if (!avr_set_ext_prog_params()) {
            avr_debugger->println("Failed to set extended parameters, retrying setup...");
            continue;
        }

        if (!avr_enter_progmode()) {
            avr_debugger->println("Failed to enter programming mode, retrying setup...");
            continue;
        }

        avr_debugger->println("Sync and programming mode successful.");
        return true;
    }
    avr_debugger->println("Failed to setup device.");
    return false;
}

static bool avr_load_address(uint16_t address) {
    uint8_t low = address & 0xFF;
    uint8_t high = (address >> 8) & 0xFF;
    // NOTE: send low byte first then high byte to match esp-idf implementation
    // (the esp-idf code passes loadAddress[1], loadAddress[0] which results
    // in low then high). Sending bytes in the wrong order caused pages to be
    // written to incorrect addresses.
    const uint8_t params[] = {low, high};
    return avr_exec_param(Cmnd_STK_LOAD_ADDRESS, params, sizeof(params), "Load address");
}

static bool avr_flash_page(uint8_t* data, size_t size) {
    uint8_t cmd_header[] = {Cmnd_STK_PROG_PAGE, (uint8_t)(size >> 8), (uint8_t)(size & 0xFF), 'F'};
    sendData(cmd_header, sizeof(cmd_header));

    // Send the page in smaller chunks to avoid overwhelming the AVR UART
    // and to better match timing of the esp-idf implementation.
    const size_t CHUNK = 64;
    size_t sent = 0;
    while (sent < size) {
        size_t take = (size - sent) > CHUNK ? CHUNK : (size - sent);
        sendData(&data[sent], take);
        sent += take;
        // small yield between chunks
        delay(2);
    }

    // short pause before EOP to ensure target has received payload
    delay(2);

    // Diagnostic: dump the packet we're about to send (header + first bytes)
    // so we can compare what was sent vs what is read back from the AVR.
    /*{
        const size_t DUMP_LEN = 64;
        size_t hdr_len = sizeof(cmd_header);
        size_t total_len = hdr_len + ((size < DUMP_LEN) ? size : DUMP_LEN);
        avr_debugger->print("Sent packet (header + first "); avr_debugger->print((total_len - hdr_len)); avr_debugger->println(" bytes):");
        // print header
        for (size_t h = 0; h < hdr_len; ++h) {
            avr_debugger->printf("%02X ", cmd_header[h]);
        }
        // print payload sample
        size_t payload_print = (size < DUMP_LEN) ? size : DUMP_LEN;
        for (size_t p = 0; p < payload_print; ++p) {
            avr_debugger->printf("%02X ", data[p]);
        }
        avr_debugger->println();
    }*/

    uint8_t eop[] = {Sync_CRC_EOP};
    sendData(eop, sizeof(eop));

    bool ok = avr_expect_in_sync_ok("Flash page", SERIAL_TIMEOUT_MS);
    if (ok) {
        // Give the AVR significantly more time to finish the actual flash
        // write before proceeding. The esp-idf version effectively waits
        // via task delays; use a longer delay here for robustness.
        delay(200);
    }
    return ok;
}

static bool avr_read_page(uint8_t* buffer, size_t size) {
    flushSerial();
    uint8_t cmd[] = {Cmnd_STK_READ_PAGE, (uint8_t)(size >> 8), (uint8_t)(size & 0xFF), 'F', Sync_CRC_EOP};
    sendData(cmd, sizeof(cmd));
    delay(2);

    uint8_t resp_header[1];
    if (receiveData(resp_header, 1, 200) != 1 || resp_header[0] != STK_INSYNC) {
        avr_debugger->printf("Read page: Failed to receive INSYNC. Got 0x%02X\n", resp_header[0]);
        return false;
    }
    if (receiveData(buffer, size, 500) != (int)size) {
        avr_debugger->println("Read page: Failed to receive full data payload.");
        return false;
    }
    uint8_t resp_footer[1];
    if (receiveData(resp_footer, 1, 200) != 1 || resp_footer[0] != STK_OK) {
        avr_debugger->printf("Read page: Failed to receive OK. Got 0x%02X\n", resp_footer[0]);
        return false;
    }
    return true;
}


// --- HEX PARSING FUNCTIONS ---

static uint8_t hex_chars_to_byte(const char* hex) {
    uint8_t result = 0;
    for (int i = 0; i < 2; ++i) {
        char c = hex[i];
        result <<= 4;
        if (c >= '0' && c <= '9') result |= (c - '0');
        else if (c >= 'a' && c <= 'f') result |= (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') result |= (c - 'A' + 10);
    }
    return result;
}

static int parse_hex_line(const char* line, uint8_t* buffer, int* address, int* data_len) {
    if (line[0] != ':') return -1;
    *data_len = hex_chars_to_byte(&line[1]);
    int addr_high = hex_chars_to_byte(&line[3]);
    int addr_low = hex_chars_to_byte(&line[5]);
    *address = (addr_high << 8) | addr_low;
    int record_type = hex_chars_to_byte(&line[7]);
    if (record_type == 0) {
        for (int i = 0; i < *data_len; ++i) {
            buffer[i] = hex_chars_to_byte(&line[9 + i * 2]);
        }
    }
    return record_type;
}

static int compute_firmware_size(const char* firmware_hex) {
    int total_size = 0;
    const char* line = firmware_hex;
    while (line && *line) {
        const char* next_line = strchr(line, '\n');
        char temp_line[256];
        if (next_line) {
            int len = next_line - line;
            if (len > 255) len = 255;
            strncpy(temp_line, line, len);
            temp_line[len] = '\0';
        } else {
            strncpy(temp_line, line, 255);
            temp_line[255] = '\0';
        }

        uint8_t data_buf[32];
        int addr = 0, data_len = 0;
        int type = parse_hex_line(temp_line, data_buf, &addr, &data_len);
        if (type == 0) {
            int end = addr + data_len;
            if (end > total_size) {
                total_size = end;
            }
        } else if (type == 1) {
            break;
        }

        line = next_line ? next_line + 1 : nullptr;
    }
    return total_size;
}

static bool populate_firmware_image(const char* firmware_hex, uint8_t* image, int image_size) {
    const char* line = firmware_hex;
    while (line && *line) {
        const char* next_line = strchr(line, '\n');
        char temp_line[256];
        if (next_line) {
            int len = next_line - line;
            if (len > 255) len = 255;
            strncpy(temp_line, line, len);
            temp_line[len] = '\0';
        } else {
            strncpy(temp_line, line, 255);
            temp_line[255] = '\0';
        }

        uint8_t data_buf[32];
        int addr = 0, data_len = 0;
        int type = parse_hex_line(temp_line, data_buf, &addr, &data_len);
        if (type == 0) {
            if (addr + data_len > image_size) {
                avr_debugger->println("HEX data exceeds allocated buffer size.");
                return false;
            }
            memcpy(&image[addr], data_buf, data_len);
        } else if (type == 1) {
            break;
        }

        line = next_line ? next_line + 1 : nullptr;
    }
    return true;
}

static uint8_t* allocate_firmware_buffer(size_t size) {
    uint8_t* buffer = (uint8_t*)heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!buffer) {
        buffer = (uint8_t*)heap_caps_malloc(size, MALLOC_CAP_8BIT);
    }
    return buffer;
}

static void log_verification_mismatch(int absolute_offset, const uint8_t* expected, const uint8_t* actual, size_t length) {
    avr_debugger->printf("Verification mismatch at absolute byte %d\n", absolute_offset);
    avr_debugger->print("Expected: ");
    for (size_t i = 0; i < length; ++i) {
        avr_debugger->printf("%02X ", expected[i]);
    }
    avr_debugger->println();
    avr_debugger->print("Actual  : ");
    for (size_t i = 0; i < length; ++i) {
        avr_debugger->printf("%02X ", actual[i]);
    }
    avr_debugger->println();
}


// --- MAIN FUNCTION ---

bool flash_avr_firmware(const char* firmware_hex, const AVRFlashConfig& config) {
    // Store configuration globally for use by other functions
    g_current_config = config;
    
    avr_debugger->println("Parsing firmware...");
    int total_size = compute_firmware_size(firmware_hex);
    if (total_size <= 0) {
        avr_debugger->println("Firmware HEX appears empty or invalid.");
        return false;
    }
    if (total_size > MAX_FIRMWARE_SIZE) {
        avr_debugger->printf("Firmware size %d exceeds maximum supported %d bytes.\n", total_size, MAX_FIRMWARE_SIZE);
        return false;
    }

    const int padded_size = ((total_size + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
    uint8_t* firmware_image = allocate_firmware_buffer(padded_size);
    if (!firmware_image) {
        avr_debugger->println("Failed to allocate firmware buffer (enable PSRAM or reduce firmware size).");
        return false;
    }
    memset(firmware_image, 0xFF, padded_size);

    if (!populate_firmware_image(firmware_hex, firmware_image, padded_size)) {
        heap_caps_free(firmware_image);
        return false;
    }

    avr_debugger->print("Parse complete. Firmware size: ");
    avr_debugger->println(total_size);

    // 2. Device setup
    if (!avr_setup_device()) {
        heap_caps_free(firmware_image);
        return false;
    }

    // 3. Write firmware
    avr_debugger->println("Writing firmware...");
    for (int i = 0; i < padded_size; i += BLOCK_SIZE) {
        int page_index = i / BLOCK_SIZE;
        uint16_t word_addr = i / 2;
        avr_debugger->printf("Writing page %d (byte offset 0x%04X, word addr 0x%04X)\n", page_index, i, word_addr);
        if (!avr_load_address(i / 2)) {
            avr_debugger->println("Failed to load address.");
            avr_leave_progmode();
            heap_caps_free(firmware_image);
            return false;
        }
        if (!avr_flash_page(&firmware_image[i], BLOCK_SIZE)) {
            avr_debugger->println("Failed to flash page.");
            avr_leave_progmode();
            heap_caps_free(firmware_image);
            return false;
        }
        avr_debugger->print(".");
    }
    avr_debugger->println("\nWrite complete.");

    // 4. Verify firmware
    avr_debugger->println("Verifying firmware...");
    uint8_t read_buffer[BLOCK_SIZE];
    for (int i = 0; i < padded_size; i += BLOCK_SIZE) {
        if (!avr_load_address(i / 2)) {
            avr_debugger->println("Failed to load address for verification.");
            avr_leave_progmode();
            heap_caps_free(firmware_image);
            return false;
        }
        if (!avr_read_page(read_buffer, BLOCK_SIZE)) {
            avr_debugger->println("Failed to read page for verification.");
            avr_leave_progmode();
            heap_caps_free(firmware_image);
            return false;
        }
        size_t chunk = ((padded_size - i) < BLOCK_SIZE) ? (padded_size - i) : BLOCK_SIZE;
        if (memcmp(&firmware_image[i], read_buffer, chunk) != 0) {
            int mismatch_index = -1;
            for (size_t j = 0; j < chunk; ++j) {
                if (firmware_image[i + j] != read_buffer[j]) {
                    mismatch_index = (int)j;
                    break;
                }
            }
            if (mismatch_index >= 0) {
                size_t preview_len = (chunk - mismatch_index < 16) ? (chunk - mismatch_index) : 16;
                log_verification_mismatch(i + mismatch_index, &firmware_image[i + mismatch_index], &read_buffer[mismatch_index], preview_len);
            }
            // Additional diagnostics: dump a larger window of expected vs actual and
            // retry a direct read after a longer delay to see if the AVR finishes
            // the internal write slightly later.
            /*{
                size_t dump_len = (chunk < 64) ? chunk : 64;
                avr_debugger->print("Dump expected (first "); avr_debugger->print(dump_len); avr_debugger->println(" bytes of page):");
                for (size_t k = 0; k < dump_len; ++k) avr_debugger->printf("%02X ", firmware_image[i + k]);
                avr_debugger->println();

                avr_debugger->print("Dump actual  (first "); avr_debugger->print(dump_len); avr_debugger->println(" bytes read):");
                for (size_t k = 0; k < dump_len; ++k) avr_debugger->printf("%02X ", read_buffer[k]);
                avr_debugger->println();

                avr_debugger->println("Waiting 500 ms and retrying read of same page...");
                delay(500);

                uint8_t retry_buffer[BLOCK_SIZE];
                if (!avr_load_address(i / 2)) {
                    avr_debugger->println("Retry: Failed to load address.");
                } else if (!avr_read_page(retry_buffer, BLOCK_SIZE)) {
                    avr_debugger->println("Retry: Failed to read page.");
                } else {
                    avr_debugger->print("Retry read (first "); avr_debugger->print(dump_len); avr_debugger->println(" bytes):");
                    for (size_t k = 0; k < dump_len; ++k) avr_debugger->printf("%02X ", retry_buffer[k]);
                    avr_debugger->println();
                }
            }*/
            avr_debugger->println("Verification failed at page.");
            avr_leave_progmode();
            heap_caps_free(firmware_image);
            return false;
        }
        avr_debugger->print(".");
    }
    avr_debugger->println("\nVerification successful.");

    // 5. Exit programming mode
    avr_leave_progmode();
    avr_debugger->println("Flashing complete.");

    heap_caps_free(firmware_image);
    return true;
}
