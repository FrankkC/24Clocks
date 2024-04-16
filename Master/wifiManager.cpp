#include <Arduino.h>
#include "WifiManager.h"
//#include "serialLink.h"

const char* ssid = "MY_WIFI_SSID";
const char* password = "***REDACTED***";

WiFiServer server(80);
WiFiClient client;
bool alreadyConnected = false;

WifiManager::WifiManager() {}

void WifiManager::init(void (*callback)()) {

    initWifi();
    callback();
    initServer();
  
}

void WifiManager::sendData(String data) {

    if (client) {
        Serial.println("sendData: " + data);
        client.println(data);
    }

    /*WIFI_SERIAL.println(cmd); // Send "AT+" command to module
    return echoFind(ack, error);*/
}

void WifiManager::initWifi() {

    Serial.println("WifiManager::initWifi()");

    WiFi.mode(WIFI_STA); //Optional
    WiFi.begin(ssid, password);
    Serial.print("Connecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());

}

void WifiManager::initServer() {
    Serial.println("WifiManager::initServer()");
    server.begin();
}

String WifiManager::readCommand() {

    String command = "";

    if (!client)
        client = server.available();  // Listen for incoming clients

    if (client) {                   // if client connected
        if (!alreadyConnected) {
            // clead out the input buffer:
            client.flush();
            Serial.println("We have a new client");
            alreadyConnected = true;
        }
        // if data available from client read and display it
        int length;
        if ((length = client.available()) > 0) {
            //str = client.readStringUntil('\n');  // read entire response
            Serial.printf("Received length %d\n", length);
            char commandChar;
            while (client.available()) {
                commandChar = (char)client.read();
                if (commandChar != '\n' && commandChar != '\r')
                    command += commandChar;
            }
        }
    }
    
    return command;

}
