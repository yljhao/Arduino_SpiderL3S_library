/*****************************************************************************
*
*  ntp.ino - Simple network time protocol client example.
*
*  Copyright (c) 2014, FunMaker Ltd.
*
*
*  This library porting from CC3000 host driver, which works with 
*  Spider_L3S WiFi module.
*
*  Spider_L3S wifi module is developed by Funmaker, we are actively 
*  involved in Taiwan maker community, and we aims to support makers 
*  to make more creative projects. 
*
*  You can support us, by buying this wifi module, and we are looking
*  forward to see your awesome projects!
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#include <Arduino.h>
#include <SPI.h>
#include <stdio.h>
#include <string.h>

#include "Spider_L3.h"
#include "NTP.h"
// GMT offset in program.
#define MY_GMT              8

// Update times per minute peroidic in ms.
#define UPDATE_TIME_PEROID  500

// Configure your WiFi module pin connection.
unsigned char WLAN_CS = 4;
unsigned char WLAN_EN = 7;
unsigned char WLAN_IRQ = 2;
unsigned char WLAN_IRQ_INTNUM = 0;


const int INDICATE_LED = 13;

// Don't forget set correct WiFi SSID and Password.
char AP_Ssid[] = {"ABCDEFGH"};
char AP_Pass[] = {"12345678"};

void Initial_Spider(void) {          
    int ret = 0;
    /* initial uart message output interface. */
    Serial.begin(115200);

    Serial.println(F("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+"));
    Serial.println(F("        Spider L3 ntp client.         "));
    Serial.println(F("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+"));

    /* initial status LED pin */
    pinMode(INDICATE_LED, OUTPUT);
    digitalWrite(INDICATE_LED, LOW);

    /* Initial Spider L3 */
    Serial.print(F("Starting Spider L3..."));
    ret = Spider_begin();
    if(ret != 0){
        while(1){};
    }
    Serial.println(F("ok"));

    // Spider L3 SPI interface initial success, indicated LED signal.
    digitalWrite(INDICATE_LED, HIGH); 
    delay(100);
    digitalWrite(INDICATE_LED, LOW);

    /* Connect to WiFi AP */
    Serial.print(F("Connect to "));
    Serial.write((unsigned char*)AP_Ssid, strlen(AP_Ssid));
    Serial.print(F(" access point..."));
    ret = Spider_Connect(3, (char*)AP_Ssid, (char*)AP_Pass);
    if(ret != 0){
        Serial.println(F("fail."));
        while(1) ;
    }
    Serial.println(F("ok"));

    /* wait connection and Get DHCP address finished */
    Serial.print(F("Waiting DHCP..."));
    while((Spider_CheckConnected() != 0) || (Spider_CheckDHCP() != 0)) {};
    Serial.println(F("ok"));

    // Spider L3 connect success, indicated LED signal.
    digitalWrite(INDICATE_LED, HIGH); 
    delay(100);
    digitalWrite(INDICATE_LED, LOW);

    /* Print device's IP address */
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

void setup() {
    Initial_Spider();

    // Set GMT +8 and ntp server address.
    NTP_begin(MY_GMT, "time.stdtime.gov.tw");
}

void loop() {

    static unsigned long update_tmr = 0;

    if(millis() > update_tmr){
        unsigned int  n_year = 0;
        unsigned char n_month = 0;
        unsigned int  n_day = 0;
        unsigned char n_hour = 0;
        unsigned char n_minute = 0;
        unsigned char n_second = 0;

        NTP_get_time(&n_year, &n_month, &n_day, &n_hour, &n_minute, &n_second);

        Serial.print("Time :");
        Serial.print(n_year, DEC);
        Serial.print("-");
        Serial.print(n_month, DEC);
        Serial.print("-");
        Serial.print(n_day, DEC);
        Serial.print(" ");
        Serial.print(n_hour, DEC);
        Serial.print(":");
        Serial.print(n_minute, DEC);
        Serial.print(":");
        Serial.print(n_second, DEC);
        Serial.println();

        update_tmr = millis() + UPDATE_TIME_PEROID;
    }
}