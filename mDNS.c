/*****************************************************************************
*
*  mDNS.c - multicast DNS library for Spider L3
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
#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include "mDNS.h"

PROGMEM unsigned char pkg_head[] = {    0x0,0x0,0x84,0x0,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x0,0x7,0x5F,0x53,0x70,0x69,0x64,0x65,
                                        0x72,0x4,0x5F,0x75,0x64,0x70,0x5,0x6C,0x6F,0x63,0x61,0x6C,0x0,0x0,0xC,0x0,0x1,0x0,0x0,
                                        0xE,0x10,0x0};

PROGMEM unsigned char pkg_mid[] = {     0x7,0x5F,0x53,0x70,0x69,0x64,
                                        0x65,0x72,0x4,0x5F,0x75,0x64,0x70,0x5,0x6C,0x6F,0x63,0x61,0x6C,0x0,0xC0,0x2A,0x0,0x21,
                                        0x0,0x1,0x0,0x0,0xE,0x10,0x0,0x8,0x0,0x0,0x0,0x0};

PROGMEM unsigned char pkg_end[] = {     0xC0,0x2A,0xC0,0x2A,0x0,0x10,
                                        0x0,0x1,0x0,0x0,0xE,0x10,0x0,0x0,0xC0,0x2A,0x0,0x1,0x0,0x1,0x0,0x0,0xE,0x10,0x0,0x4};

long mdns_socket = -1;
sockaddr    mdns_addr;
int mDNS_begin(){
    // Get socket from spider L3.
    mdns_socket = -1;
    mdns_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(mdns_socket < 0){
        mdns_socket = -1;
        return -1;
    }

    memset(&mdns_addr, 0, sizeof(mdns_addr));
    mdns_addr.sa_family = AF_INET;
    SET_mDNS_ADD(mdns_addr);

    return 0;
}

int mDNS_close(){
    int ret;
    ret = closesocket(mdns_socket);
    mdns_socket = -1;
    return ret;
}

int mDNS_Advertiser(char* device_name, unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned char ip4, unsigned int port){

    unsigned char mdns_adresponse[130];
    int  mdns_adr_len = 0;

    memset(mdns_adresponse, 0, sizeof(mdns_adresponse));
    mdns_adr_len = 0;

    memcpy_P(&mdns_adresponse[mdns_adr_len], pkg_head, sizeof(pkg_head));
    mdns_adr_len += sizeof(pkg_head);

    unsigned char total_len = 20;
    unsigned char dn_len = 0;
    dn_len = strlen(device_name);
    total_len = total_len + dn_len + 1;

    mdns_adresponse[mdns_adr_len] = total_len;
    mdns_adr_len += 1;

    mdns_adresponse[mdns_adr_len] = dn_len;
    mdns_adr_len += 1;

    memcpy(&mdns_adresponse[mdns_adr_len], device_name, dn_len);
    mdns_adr_len += dn_len;

    memcpy_P(&mdns_adresponse[mdns_adr_len], pkg_mid, sizeof(pkg_mid));
    mdns_adr_len += sizeof(pkg_mid);

    mdns_adresponse[mdns_adr_len] = ((port >> 8) & 0xFF);
    mdns_adr_len += 1;
    mdns_adresponse[mdns_adr_len] = (port & 0xFF);
    mdns_adr_len += 1;

    memcpy_P(&mdns_adresponse[mdns_adr_len], pkg_end, sizeof(pkg_end));
    mdns_adr_len += sizeof(pkg_end);

    mdns_adresponse[mdns_adr_len] = ip1;
    mdns_adr_len += 1;
    mdns_adresponse[mdns_adr_len] = ip2;
    mdns_adr_len += 1;
    mdns_adresponse[mdns_adr_len] = ip3;
    mdns_adr_len += 1;
    mdns_adresponse[mdns_adr_len] = ip4;
    mdns_adr_len += 1;

    sendto(mdns_socket, mdns_adresponse, mdns_adr_len, 0, &mdns_addr, sizeof(mdns_addr));

}

