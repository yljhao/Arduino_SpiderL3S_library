/*****************************************************************************
*
*  SimpleHttpServer.ino - Simple HTTP web server, user can change 
*  GPIO status from web page.
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
#include <stdio.h>
#include <string.h>

#include "Spider_L3.h"
#include "WebServer.h"

// Configure your WiFi module pin connection.
#if 1
unsigned char WLAN_CS = 4;
unsigned char WLAN_EN = 7;
unsigned char WLAN_IRQ = 2;
unsigned char WLAN_IRQ_INTNUM = 0;
#else
unsigned char WLAN_CS = 24;
unsigned char WLAN_EN = 25;
unsigned char WLAN_IRQ = 21;
unsigned char WLAN_IRQ_INTNUM = 2;
#endif

// Don't forget set correct WiFi SSID and Password.
char AP_Ssid[] = {"WIFISSID"};
char AP_Pass[] = {"12345678"};

const int INDICATE_LED = 13;

const int CONTROL_GPIO = 8;

unsigned char CONTROL_STATE = LOW;


void setup() {          

    int ret = 0;

    /* initial uart debug output interface. */
    Serial.begin(115200);

    Serial.println(F("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+"));
    Serial.println(F("     Spider L3 simple http server.    "));
    Serial.println(F("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+"));

    /* initial status LED pin */
    pinMode(INDICATE_LED, OUTPUT);
    digitalWrite(INDICATE_LED, LOW);

    CONTROL_STATE = LOW;
    digitalWrite(INDICATE_LED, CONTROL_STATE);

    /* Initial monitor IO pin */
    pinMode(8, INPUT);

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

#define INIT_SERVER             0x01
#define WAIT_CONNECT            0x02
#define HANDLE_REQUEST          0x03
#define HANDLE_RESPONSE         0x04
#define CLOSE_CLIENT_CONN       0x05


void WebService(void){
    static unsigned char app_st = INIT_SERVER;
    static int http_server_socket = -1;
    static int client_socket = -1;
    static unsigned long task_timer = 0;

    char data_pool[100];
    int ret = 0;

    char method[10];
    char file[30];
    char content[100];

    if(millis() > task_timer){
        switch(app_st){
            case INIT_SERVER:
                http_server_socket = WebServer_begin(80);
                if(http_server_socket >= 0){
                    app_st = WAIT_CONNECT;
                }
            break;
            case WAIT_CONNECT:
                client_socket = WebServer_wait_connect(http_server_socket);
                if(client_socket >= 0){
                    Serial.println(F("Incomming connection."));
                    app_st = HANDLE_REQUEST;
                }
            break;
            case HANDLE_REQUEST:
                Serial.println(F("Handle incomming request."));

                memset(method, 0, sizeof(method));
                memset(file, 0, sizeof(file));
                memset(content, 0, sizeof(content));

                WebServer_process_request(client_socket, method, sizeof(method), file, sizeof(file), content, sizeof(content));

                Serial.print(F("method:"));
                Serial.write((unsigned char*)method, strlen(method));
                Serial.println();
                Serial.print(F("filename:"));
                Serial.write((unsigned char*)file, strlen(file));
                Serial.println();
                Serial.print(F("Content:"));
                Serial.write((unsigned char*)content, strlen(content));
                Serial.println();
                app_st = HANDLE_RESPONSE;
                break;

            case HANDLE_RESPONSE:

                if(strstr_P(method, PSTR("POST")) != 0){
                    if(strstr_P(content, PSTR("HIGH")) != 0){
                        CONTROL_STATE = HIGH;
                    }

                    if(strstr_P(content, PSTR("LOW")) != 0){
                        CONTROL_STATE = LOW;
                    }
                }
                
                if(strstr_P(file, PSTR("favicon")) != 0){
                    WebServer_put_notfound(client_socket);
                    app_st = CLOSE_CLIENT_CONN;
                    break;
                }

                if((strstr_P(file, PSTR("/")) != 0) || (strstr_P(file, PSTR("index.html")) != 0)){
                    /* Put HTML header */
                    memset(data_pool, 0, sizeof(data_pool));
                    strncpy_P(data_pool, PSTR("HTTP/1.1 200 OK\r\n"), sizeof(data_pool));
                    WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                    memset(data_pool, 0, sizeof(data_pool));
                    strncpy_P(data_pool, PSTR("Content-Type: text/html\r\n"), sizeof(data_pool));
                    WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                    memset(data_pool, 0, sizeof(data_pool));
                    strncpy_P(data_pool, PSTR("Connection: close\r\n\r\n"), sizeof(data_pool));
                    WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                    /* PUT Message */
                    memset(data_pool, 0, sizeof(data_pool));
                    strncpy_P(data_pool, PSTR("<html>\r\n<body>\r\n\r\n"), sizeof(data_pool));
                    WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                    ret = snprintf(data_pool, sizeof(data_pool), "Input variable :%s\r\n", ((CONTROL_STATE == HIGH) ? "HIGH" : "LOW"));
                    WebServer_put_response(client_socket, data_pool, ret);

                    memset(data_pool, 0, sizeof(data_pool));
                    strncpy_P(data_pool, PSTR("<form method=\"post\">"), sizeof(data_pool));
                    WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                    memset(data_pool, 0, sizeof(data_pool));
                    strncpy_P(data_pool, PSTR("<input type=\"hidden\" name=\"var\" value=\"HIGH\">"), sizeof(data_pool));
                    WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                    memset(data_pool, 0, sizeof(data_pool));
                    strncpy_P(data_pool, PSTR("<input type=\"submit\" style=\"height:30px; width:100px\" value=\"HIGH\">\r\n</form>\r\n"), sizeof(data_pool));
                    WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                    memset(data_pool, 0, sizeof(data_pool));
                    strncpy_P(data_pool, PSTR("<form method=\"post\">"), sizeof(data_pool));
                    WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                    memset(data_pool, 0, sizeof(data_pool));
                    strncpy_P(data_pool, PSTR("<input type=\"hidden\" name=\"var\" value=\"LOW\">"), sizeof(data_pool));
                    WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                    memset(data_pool, 0, sizeof(data_pool));
                    strncpy_P(data_pool, PSTR("<input type=\"submit\" style=\"height:30px; width:100px\" value=\"LOW\">\r\n</form>\r\n"), sizeof(data_pool));
                    WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                    memset(data_pool, 0, sizeof(data_pool));
                    strncpy_P(data_pool, PSTR("</html>\r\n</body>\r\n\r\n"), sizeof(data_pool));
                    WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                }

                task_timer = millis() + 250;
                app_st = CLOSE_CLIENT_CONN;
            break;

            case CLOSE_CLIENT_CONN:
                Serial.println(F("Close connection."));
                WebServer_close_connect(client_socket);
                client_socket = -1;
                app_st = WAIT_CONNECT;
            break;

        }
    }
}

void loop() {
    WebService();
}
