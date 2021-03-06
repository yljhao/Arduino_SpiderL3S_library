/*****************************************************************************
*
*  SimpleHttpGet.ino - Simple Yahoo weather RSS information get 
*  example through Yahoo API interface.
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
#include "WebClient.h"

// Configure your WiFi module pin connection.
unsigned char WLAN_CS = 4;
unsigned char WLAN_EN = 7;
unsigned char WLAN_IRQ = 2;
unsigned char WLAN_IRQ_INTNUM = 0;


// Don't forget set correct WiFi SSID and Password.
char AP_Ssid[] = {"QWERTYUI"};
char AP_Pass[] = {"12345678"};

const int INDICATE_LED = 13;

void setup() {          

    int ret = 0;
    unsigned long tmr = 0;

    /* initial uart debug output interface. */
    Serial.begin(115200);
    
    // Wait 2 Sec if serial is not ready.
    tmr = millis() + 2000;
    while(!Serial){
        if(millis() > tmr) break;
    }

    Serial.println(F("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+"));
    Serial.println(F("     Spider L3 simple http client.    "));
    Serial.println(F("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+"));

    /* initial status LED pin */
    pinMode(INDICATE_LED, OUTPUT);
    digitalWrite(INDICATE_LED, LOW);

    /* Initial Spider L3 */
    Serial.print(F("Starting Spider L3..."));
    ret = Spider_begin();
    if(ret != 0){
        Serial.println(F("fail, please check connection pin."));
        while(1) ;
    }
    Serial.println(F("ok"));

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
    while((Spider_CheckConnected() != 0) || (Spider_CheckDHCP() != 0)) ;
    Serial.println(F("ok"));

    tNetappIpconfigRetArgs inf;
    netapp_ipconfig(&inf);

    Serial.print(F("Device IP address:"));
    Serial.print(inf.aucIP[3] ,DEC);
    Serial.print(F("."));
    Serial.print(inf.aucIP[2] ,DEC);
    Serial.print(F("."));
    Serial.print(inf.aucIP[1] ,DEC);
    Serial.print(F("."));
    Serial.print(inf.aucIP[0] ,DEC);
    Serial.print(F("\r\n"));

}

/* For contex switch use */
#define CONNECT_BEGIN       0x01
#define CONNECT_SERVER      0x02
unsigned char app_st = CONNECT_BEGIN;

/* Target URL link */
char api_url_link[] = {"http://weather.yahooapis.com/forecastrss?w=2306179&u=c"};

/* Target server's information */
Host_Info host_inf;
unsigned char ip[4];

/* Buffer for storage information from webpage */
char ret_data[100];
int ret = 0;

/* Timer register */
unsigned long task_timer = 0;
unsigned long break_tmr = 0;

unsigned char conn_count = 0;

/* Counting send http request times */
int get_count = 0;


void loop() {
    if(millis() > task_timer){
        switch(app_st){

            case CONNECT_BEGIN:

                /* Use gethostbyname api to get target server's IP address. */
                Serial.print(F("Get target server IP..."));
                memset(ip, 0, sizeof(ip));
                ret = WebClient_Get_HostIP(api_url_link, &ip[0], &ip[1], &ip[2], &ip[3]);

                if(ret != 0){
                    task_timer = millis() + 1000;
                    Serial.println(F("failed"));
                    break;
                }
                Serial.println(F("ok"));

                Serial.print(F("Target server IP address:"));
                Serial.print(ip[0] ,DEC);
                Serial.print(F("."));
                Serial.print(ip[1] ,DEC);
                Serial.print(F("."));
                Serial.print(ip[2] ,DEC);
                Serial.print(F("."));
                Serial.print(ip[3] ,DEC);
                Serial.print(F("\r\n"));

                /* Get ip address, switch to CONNECT_SERVER status*/
                //get_count = 0;
                app_st = CONNECT_SERVER;
                task_timer = millis() + 100;
                break;

            case CONNECT_SERVER:

                /* Initial host_inf socket option */
                Serial.print(F("Set target server's socket...\r\n"));
                Serial.print(F("IP addr..."));
                Serial.print(ip[0],DEC);
                Serial.print(F("."));
                Serial.print(ip[1],DEC);
                Serial.print(F("."));
                Serial.print(ip[2],DEC);
                Serial.print(F("."));
                Serial.print(ip[3],DEC);
                Serial.print(F(":"));
                Serial.print(80,DEC);
                Serial.print(F("..."));
                memset(&host_inf, 0, sizeof(host_inf));
                ret = WebClient_Begin_IP(&host_inf, ip[0],  ip[1],  ip[2],  ip[3], 80);
                if(ret != 0){
                    Serial.println(F("failed"));
                    WebClient_Close(&host_inf);
                    task_timer = millis() + 1000;
                    break;
                }
                Serial.println(F("ok"));

                Serial.print(F("Host socket number:"));
                Serial.println(host_inf.host_socket, DEC);

                /* Connect to WebServer */
                Serial.print(F("Connect to target server..."));
                ret = WebClient_Connect(&host_inf);
                if(ret != 0){
                    app_st = CONNECT_BEGIN;
                    Serial.println(F("failed"));
                    WebClient_Close(&host_inf);
                    task_timer = millis() + 1000;
                    break;
                }
                Serial.println(F("ok"));

                if(conn_count > 29){
                    app_st = CONNECT_BEGIN;
                    conn_count = 0;
                }
                else{
                    conn_count++;
                }

                /* Send http request to WebServer */
                Serial.print(F("Send request to target server..."));
                ret = WebClient_SendRequest(&host_inf, api_url_link);
                if(ret != 0){
                    Serial.println(F("failed"));
                    /* Send request error, close connection. */
                    WebClient_Close(&host_inf);
                    task_timer = millis() + 1000;
                    break;
                }
                Serial.println(F("ok"));

                /* Print request success count. */
                get_count++;
                Serial.print(F("No."));
                Serial.print(get_count, DEC);
                Serial.println(F(" http request send."));

                /* Get information, timeout is 1500ms */
                break_tmr = millis() + 800;
                while(1){
                    /* Check data available*/
                    if(WebClient_DataAvailable(&host_inf) > 0){
                        /* Receiving data... */
                        memset(ret_data, 0, sizeof(ret_data));
                        ret = WebClient_RecvData(&host_inf, ret_data, sizeof(ret_data));
                        if(ret > 0){
                            /* Received success, refresh timeout timer */
                            break_tmr = millis() + 1500;

                            /* Do information parsing */
                            char* str1;
                            str1 = strstr(ret_data, "temp=\"");
                            if(str1 != 0){
                                digitalWrite(INDICATE_LED, HIGH);
                                
                                char* str2 = 0;
                                int temp = 0;
 
                                str1 += strlen("temp=\"");
                                str2 = strstr(str1, "\"");
                                *str2 = 0;
                                sscanf(str1, "%d", &temp);

                                /* Print tempecture */
                                Serial.print(F("Tempecture of Taipei city:"));
                                Serial.print(temp, DEC);
                                Serial.println();

                                digitalWrite(INDICATE_LED, LOW);
                            }

                        }
                    }

                    /* Check timeout */
                    if(millis() > break_tmr){
                        Serial.println(F("Timeout, breaking recive loop."));
                        break;
                    } 
                }

                /* http get complete, close connection */
                WebClient_Close(&host_inf);
                Serial.println(F("Connection closed."));

                /* Wait 1000ms and do http get again */
                task_timer = millis() + 100;
                break;
        }
    }
}
