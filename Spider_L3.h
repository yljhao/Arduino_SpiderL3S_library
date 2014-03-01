#ifndef __SPIDER_L3_H__
#define __SPIDER_L3_H__
/*****************************************************************************
*
*  Spider_L3.h.
*  
*****************************************************************************/
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

#ifdef  __cplusplus
}
#endif

#endif //__SPIDER_L3_H__