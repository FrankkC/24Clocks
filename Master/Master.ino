/*
MASTER:
Serial1 --> Slave 1
Serial2 --> Slave 2
SerialZ --> Serial Monitor

SLAVE 1:
Serial --> Serial Monitor
Serial3 --> Master

SLAVE 2:
Serial --> Serial Monitor
Serial3 --> Master
*/

/*
TODO:
Step2: Fix bug Rotazione lancetta tra 2 e 3 alla sesta (settima) iterazione del counter
Step3: Funzionalità per regolare la posizione delle lancette e comunicazione con WiFi e App Android
*/

#include <Arduino.h>
#include "wifiManager.h"
#include "serialLink.h"

int timer = 0;
bool countMode = false;
// TODO: Gestire il cambio di modalità
bool timeMode = false;

int32_t timeOffset = 0;
int minutesSinceMidnight = 0;

String commandBuffer;

void setup() {

    Serial.begin(115200);
    delay(1000);
    Serial.println("Launching Master");

    SerialLink::init();
    WifiManager::init();

}

void loop() {

    /*if (!countMode && millis() > 10000) {
        countMode = true;
    }*/

    /*if (countMode && timer != (millis()/1000)%10) {
        timer = (millis()/1000)%10;
        char time[4];
        sprintf (time, "%d%d%d%d", timer, timer, timer, timer);
        setDisplayTime(time);
    }

    int newMinutesSinceMidnight = (timeOffset + (millis()/1000)%86400)/60;
    if (timeMode && newMinutesSinceMidnight != minutesSinceMidnight) {
        minutesSinceMidnight = newMinutesSinceMidnight;
        setTime(minutesSinceMidnight);
        // Qui ci vorrebbe uno setDisplayCurrentTime
        int minutes = minutesSinceMidnight%60;
        int hours = (minutesSinceMidnight-minutes)/60;

        char time[4];
        sprintf (time, "%02d%02d", hours, minutes);
        setDisplayTime(time);
    }*/

    handleWifiCommand();

}

void handleWifiCommand() {

    String command = WifiManager::readCommand();

    if (command != "") {

        Serial.println("handleWifiCommand: " + command);

        if (command.indexOf("SETTIME=") != -1) {
            String newTime = command.substring(command.indexOf("TIME=") + 5, command.indexOf("TIME=") + 9);

            // Converto il time in secondi dalla mezzanotte
            int32_t tempIntTime = atoi(newTime.c_str());
            int32_t minutes = tempIntTime % 100;
            int32_t hours = (tempIntTime - minutes) / 100;
            int32_t newTimeInSeconds = minutes * 60 + hours * 60 * 60;

            // Sottraggo i secondi dall'accensione%24oreInSecondi
            // setto timeoffset a questo valore in secondi
            timeOffset = newTimeInSeconds - (millis()/1000)%86400;

            // metto timeMode = true in modo da avviare l'update della posizione delle lancette
            timeMode = true;
            countMode = false;

            //setDisplayTime(newTime.c_str());
            WifiManager::sendData("SET TIME OK");
        } else if (command.indexOf("SETHOME") != -1) {
            timeMode = false;
            countMode = false;
            setHome();
            WifiManager::sendData("SET HOME OK");
        } else if (command.indexOf("SETZERO") != -1) {
            timeMode = false;
            countMode = false;
            setDisplayTime("0000");
            WifiManager::sendData("SET ZERO OK");
        } else if (command.indexOf("SETCOUNT=1") != -1) {
            timeMode = false;
            countMode = true;
            WifiManager::sendData("SET COUNT MODE ON OK");
        } else if (command.indexOf("SETCOUNT=0") != -1) {
            countMode = false;
            WifiManager::sendData("SET COUNT MODE OFF OK");
        } else if (command.indexOf("ECHO") != -1) {
            WifiManager::sendData("ECHO OK");
        }

    }

}

void setTime(int32_t minutesSinceMidnight) {
    // timeOffset è la differenza tra il tempo che vogliamo impostare e quanto riportato da millis().
    // il modulo 86400 serve a riportare millis() a un valore compreso in una giornata (se il dispositivo è acceso da più di 24h)
    timeOffset = minutesSinceMidnight * 60 - (millis()/1000)%86400;
}

int getTime() {
    return timeOffset + (millis()/1000)%86400;
}


void setDisplayTime(const char* time) {
    // Adesso possiamo usare il Serial Print locale
    Serial.println("setDisplayTime: " + String(time));
    // Aggiungere comando per il secondo slave
    SerialLink::sendCommand("SETTIME=" + String(time));
}

void setHome() {
    Serial.println("Going home...");
    SerialLink::sendCommand("SETHOME");
}
