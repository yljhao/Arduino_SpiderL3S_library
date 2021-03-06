/*****************************************************************************
*
*  Spider_L3.h - WiFi initial function.
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
#ifndef __SPIDER_L3_H__
#define __SPIDER_L3_H__

#include "cc3000_common.h"
#include "evnt_handler.h"
#include "hci.h"
#include "host_driver_version.h"
#include "netapp.h"
#include "nvmem.h"
#include "security.h"
#include "socket.h"
#include "wlan.h"
#include "spider_spi.h"

#define SPIDER_ARP_FLAG_INIT		1
#define SPIDER_ARP_FLAG_PROC		2
#define SPIDER_ARP_FLAG_DONE		3

#ifdef  __cplusplus
extern "C" {
#endif

extern int Spider_begin(void);

extern int Spider_close(void);

extern int Spider_Connect(unsigned long sec_mode, char* tar_ssid, char* tar_password);

extern int Spider_CheckDHCP(void);

extern int Spider_CheckConnected(void);

extern int Spider_Disconnect(void);

/* Smart config functions in testing, don't use it. */

extern int Spider_SmartConfig(void);

extern int Spider_SmartConfig_AckFinish(char* ack_msg);

extern int Spider_AutoConnect(void);

extern void Spider_Start_ARP_EVENT(void);

extern unsigned char Spider_Chk_ARP_EVENT(void);

#ifdef  __cplusplus
}
#endif

#endif //__SPIDER_L3_H__