
#include <Arduino.h>
#include "WifiManager.h"
#include "serialLink.h"

WifiManager::WifiManager() {}

void WifiManager::init() {

    WIFI_SERIAL.begin(115200);

    bool success = true;
    
    success &= initWifi();

    if (success) {
        success &= initServer();
    }

    if (success) {
        SerialLink::sendLog("Ready >");
    }
  
}

bool WifiManager::sendCommand(String cmd, String ack = "", String error = "") {
    WIFI_SERIAL.println(cmd); // Send "AT+" command to module
    return echoFind(ack, error);
}

bool WifiManager::echoFind(String ok, String error) {
  
    if (ok == "" && error == "")
        return true;

    byte ok_current_char = 0;
    byte ok_length = ok.length();

    byte error_current_char = 0;
    byte error_length = error.length();

    String response;

    long deadline = millis() + TIMEOUT;
    while (millis() < deadline) {
        if (WIFI_SERIAL.available()) {
            char ch = WIFI_SERIAL.read();
            //MASTER_SLAVE_SERIAL.write(ch);

            if (ok != "") {
                if (ch == ok[ok_current_char]) {
                    if (++ok_current_char == ok_length) {
                        //MASTER_SLAVE_SERIAL.println();
                        return true;
                    }
                } else {
                    ok_current_char = 0;
                }
            }
 
            if (error != "") {
                if (ch == error[error_current_char]) {
                    if (++error_current_char == error_length) {
                        //MASTER_SLAVE_SERIAL.println();
                        return false;
                    }
                } else {
                    error_current_char = 0;
                }
            }

        }
    }
    return false; // Timed out
}

void WifiManager::sendData(String str) {
    String len = "";
    len += str.length();
    sendCommand("AT+CIPSEND=0," + len, "OK"); // Setup   command to send str data length to channel 0
    sendCommand(str, "OK"); //Send   String str with 0 closing the transmission and return to AT mode
}

bool WifiManager::initWifi() {

    SerialLink::sendLog("WifiManager::initWifi()");

    bool success = true;

    //MASTER_SLAVE_SERIAL.print("Initializing WiFi");


    // Non sempre il reset esce in ready. Rimane appeso e va in timeout.
    // Per ora lo disattivo.
    // success &= sendCommand("AT+RST", "Technology");
    // MASTER_SLAVE_SERIAL.println("Success:");
    // MASTER_SLAVE_SERIAL.println(success);

    // Controllare il funzionamento di echoFind
    // Implementare controllo errore in echoFind

    success &= sendCommand("AT+CWMODE=1","OK","ERROR");
    if (success) {
        //MASTER_SLAVE_SERIAL.print(".");
    } else {
        return false;
    }

    success &= sendCommand("AT+CWLAP","OK","ERROR");
    if (success) {
        //MASTER_SLAVE_SERIAL.print(".");
    } else {
        return false;
    }

    success &= sendCommand("AT+CWJAP=\"MY_WIFI_SSID\",\"***REDACTED***\"","OK");
    if (success) {
        //MASTER_SLAVE_SERIAL.print(".");
    } else {
        return false;
    }

    success &= sendCommand("AT+CIFSR", "OK");
    if (success) {
        //MASTER_SLAVE_SERIAL.print(".");
    } else {
        return false;
    }

    success &= sendCommand("AT+CIPMUX=1","OK");
    if (success) {
        //MASTER_SLAVE_SERIAL.println(".");
        return true;
    } else {
        return false;
    }

}

bool WifiManager::initServer() {

    SerialLink::sendLog("WifiManager::initServer()");

    //MASTER_SLAVE_SERIAL.print("Initializing Server");

    bool success = sendCommand("AT+CIPSERVER=1,80","OK");
    if (success) {
        //MASTER_SLAVE_SERIAL.println(".");
        return true;
    } else {
        return false;
    }

}

String WifiManager::readCommand() {

    String incomingString = "";

    while (WIFI_SERIAL.available()) {
        incomingString = WIFI_SERIAL.readString();
    }

    return incomingString;

}
