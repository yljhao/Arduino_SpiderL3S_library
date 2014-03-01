/*

Example shows how to use smart config setting SSID and password.

*/

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "Spider_L3.h"

const int INDICATE_LED = 13;
const int SMART_CONFIG_PIN = 8;

void setup() {          
    int ret = 0;
    /* initial uart debug output interface. */
    Serial.begin(115200);

    Serial.println(F("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+"));
    Serial.println(F("     Spider L3 smart config demo.     "));
    Serial.println(F("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+"));

    /* initial status LED pin */
    pinMode(INDICATE_LED, OUTPUT);
    digitalWrite(INDICATE_LED, LOW);

    /* initial smart config enable pin */
    pinMode(SMART_CONFIG_PIN, INPUT);

    /* Initial Spider L3 spi interface*/
    Serial.print(F("Starting Spider L3..."));
    ret = Spider_begin();
    if(ret != 0){
        while(1);
    }
    Serial.println(F("ok"));

    // Spider L3 SPI interface initial success, indicated LED signal.
    digitalWrite(INDICATE_LED, HIGH); 
    delay(100);
    digitalWrite(INDICATE_LED, LOW);

    /* Checking GPIO pin selecting use smart config setting connect profile or connect to network */ 
    Serial.print(F("Check select pin:"));
    Serial.print(SMART_CONFIG_PIN, DEC);
    Serial.print(F(",level..."));

    if(digitalRead(SMART_CONFIG_PIN) == HIGH){
        Serial.println(F("HIGH"));
        Serial.print(F("Starting smart config..."));
        Spider_SmartConfig();
    }
    else{
        Serial.println(F("LOW"));
        Serial.print(F("Starting auto connect..."));
        Spider_AutoConnect();
    }
    Serial.println(F("ok"));

    Serial.print(F("Waiting DHCP..."));
    /* wait connection and Get DHCP address finished */
    while((Spider_CheckConnected() != 0) || (Spider_CheckDHCP() != 0));
    Serial.println(F("ok"));

    // Spider L3 connect success, indicated LED signal.
    digitalWrite(INDICATE_LED, HIGH); 
    delay(100);
    digitalWrite(INDICATE_LED, LOW);

    /* Do smart config ack */
    Spider_SmartConfig_AckFinish("SpiderL3");

    /* show system IP address */
    tNetappIpconfigRetArgs inf;
    netapp_ipconfig(&inf);

    Serial.print(F("Device's IP address:"));
    Serial.print(inf.aucIP[3] ,DEC);
    Serial.print(F("."));
    Serial.print(inf.aucIP[2] ,DEC);
    Serial.print(F("."));
    Serial.print(inf.aucIP[1] ,DEC);
    Serial.print(F("."));
    Serial.print(inf.aucIP[0] ,DEC);
    Serial.print(F("\r\n"));
}

void loop() {
}
