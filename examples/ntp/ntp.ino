/*

Example shows how to use smart config setting SSID and password.

*/

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "Spider_L3.h"

// Configure your WiFi module pin connection.
#if 0
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

const int INDICATE_LED = 13;

// Don't forget set correct WiFi SSID and Password.
char AP_Ssid[] = {"WIFISSID"};
char AP_Pass[] = {"12345678"};

int Initial_get_time(void);

void Initial_Spider(void) {          
    int ret = 0;
    /* initial uart message output interface. */
    Serial.begin(115200);

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
    Serial.print(F("Connect to AP..."));
    ret = Spider_Connect(3, AP_Ssid, AP_Pass);
    if(ret != 0){
        while(1){};
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
    Initial_get_time();
}

void Find_Me(void){
    /* Timer register */
    static unsigned long tmr = 0;

    /* Simple mdnsAdvisor function */
    char mdns_msg[10];
    memset(mdns_msg, 0, sizeof(mdns_msg));

    if(millis() > tmr){
        strncpy_P(mdns_msg, PSTR("WiFiSW"), sizeof(mdns_msg));
        mdnsAdvertiser(1, (char*)mdns_msg, strlen(mdns_msg));
        tmr = millis() + 1000;
    }
}
const char ntp_server[] = "clock.stdtime.gov.tw";
const unsigned int ntp_port = 123; 

long ntp_socket = 0xFFFFFFFF;
sockaddr    host_addr;
sockaddr    recv_addr;
socklen_t   recv_addr_l;
int Initial_get_time(void){
    int ret = -1;
    long ntp_timeout = 0;

    union {
        unsigned long ulip;
        unsigned char ucip[4];
    }tar_ip;

    union{
        unsigned short  usport;
        unsigned char   ucport[2];
    }tar_port;

    memset(&host_addr, 0, sizeof(sockaddr));

    // Get ntp server's IP from host name.
    ret = gethostbyname((char*)ntp_server, strlen(ntp_server), &(tar_ip.ulip));
    if(ret < 0){
        return -1;
    }

    // Get socket from spider L3.
    ntp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(ntp_socket < 0){
        ntp_socket = 0xFFFFFFFF;
        return -2;
    }

    // Setting BSD socket timeout.
    ntp_timeout = 1000;
    ret = setsockopt(ntp_socket, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &ntp_timeout, sizeof(ntp_timeout));
    if(ret < 0){
        closesocket(ntp_socket);
        ntp_socket = 0xFFFFFFFF;
        return -3;
    }

    // Setting BSD like socket setting.
    host_addr.sa_family = AF_INET;
    tar_port.usport = ntp_port;

    host_addr.sa_data[0] = tar_port.ucport[1];
    host_addr.sa_data[1] = tar_port.ucport[0];

    host_addr.sa_data[2] = tar_ip.ucip[3];
    host_addr.sa_data[3] = tar_ip.ucip[2];
    host_addr.sa_data[4] = tar_ip.ucip[1];
    host_addr.sa_data[5] = tar_ip.ucip[0];

}


#define ntp_header_0        0b00100011
#define SEVENTY_YEAR_SEC    2208988800UL
#define UNIX_TIME_YEARS     1970
#define MY_GMT              8
int get_time(unsigned char *hour, unsigned char *minute, unsigned char *second){
    int ret = -1;
    unsigned char msg[48];
    unsigned long timestamp_now = 0;

    Serial.println("Starting get time...");
    memset(msg, 0, sizeof(msg));
    msg[0] = ntp_header_0;
    Serial.println("Starting sending time request...");
    sendto(ntp_socket, msg, sizeof(msg), 0, &host_addr, sizeof(sockaddr));
    Serial.println("Finished sending time request...");

    memset(&recv_addr, 0, sizeof(sockaddr));
    recv_addr_l = 0;
    Serial.println("Starting receiving time data...");
    ret = recvfrom(ntp_socket, msg, sizeof(msg), 0, &recv_addr, &recv_addr_l);

    // Caculate timestamp rightnow
    timestamp_now = ((unsigned long)msg[40] << 24) | ((unsigned long)msg[41] << 16) | ((unsigned long)msg[42] << 8) | ((unsigned long)msg[43]);
    timestamp_now -= SEVENTY_YEAR_SEC;

    *second = timestamp_now % 60;

    *minute = (timestamp_now / 60) % 60;

    *hour =  ((MY_GMT +(timestamp_now / 3600)) % 24);

    return ret;
}

#define UPDATE_TIME_PEROID  500

unsigned long update_tmr = 0;

void loop() {
    //Find_Me();
    
    if(millis() > update_tmr){

        unsigned char n_hour = 0;
        unsigned char n_minute = 0;
        unsigned char n_second = 0;

        get_time(&n_hour, &n_minute, &n_second);

        Serial.print("Time :");
        Serial.print(n_hour, DEC);
        Serial.print(":");
        Serial.print(n_minute, DEC);
        Serial.print(":");
        Serial.print(n_second, DEC);
        Serial.println();

        update_tmr = millis() + UPDATE_TIME_PEROID;
    }
}
