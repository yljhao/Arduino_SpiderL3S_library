/*****************************************************************************
*
*  Spider_L3.h  - WiFi initial function.
*
*****************************************************************************/
#include <Arduino.h>
#include "Spider_L3.h"

// Simple Config Prefix
static const char SmartConfig_prefix[] = {'T', 'T', 'T'};

//AES key "smartconfigAES16"
static const unsigned char SmartConfig_key[] = {0x73,0x6d,0x61,0x72,0x74,0x63,0x6f,0x6e,0x66,0x69,0x67,0x41,0x45,0x53,0x31,0x36};

// CC3000 system flag
static volatile unsigned char HW_Initialed = 0;

// CC3000 async callback flag
static volatile unsigned char SmartConfigFinished = 0;
static volatile unsigned char StopSmartConfig = 0;
static volatile unsigned char SpiderConnected = 0;
static volatile unsigned char SpiderDHCP = 0;
static volatile unsigned char SpiderCanShutDown = 0;

/*-------------------------------------------------------------------

    The TI library calls this routine when asynchronous events happen.
    
    For example you tell the CC3000 to turn itself on and connect
    to an access point then your code can go on to do its own thing.
    When the CC3000 is done configuring itself (e.g. it gets an IP
    address from the DHCP server) it will call this routine so you
    can take appropriate action.    

---------------------------------------------------------------------*/

void Spider_AsyncCallback(long lEventType, char * data, unsigned char length)
{
  (void)(data);
  (void)(length);
    switch (lEventType){
        case HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE:
            SmartConfigFinished = 1;
            StopSmartConfig     = 1;
            break;
            
        case HCI_EVNT_WLAN_UNSOL_CONNECT:
            SpiderConnected = 1;
            break;
            
        case HCI_EVNT_WLAN_UNSOL_DISCONNECT:
            SpiderConnected = 0;
            SpiderDHCP      = 0;     
            break;
            
        case HCI_EVNT_WLAN_UNSOL_DHCP:
            SpiderDHCP = 1;
            break;
            
        case HCI_EVENT_CC3000_CAN_SHUT_DOWN:
            SpiderCanShutDown = 1;
            break;
                        
        default:
            break;      
    }
}

/*-------------------------------------------------------------------

    The TI library calls these routines on CC3000 startup.
    
    This library does not send firmware, driver, or bootloader patches
    so we do nothing and we return NULL.

---------------------------------------------------------------------*/

char *SendFirmwarePatch(unsigned long *Length)
{
    *Length = 0;
    return NULL;
}

char *SendDriverPatch(unsigned long *Length) 
{
    *Length = 0;
    return NULL;
}

char *SendBootloaderPatch(unsigned long *Length)
{
    *Length = 0;
    return NULL;
}
/*------------------------------------------------------------------------
  Spider_begin
  
  Initial Spider L3, 
  
  return  0, Initial success.
  
-----------------------------------------------------------------------*/
int Spider_begin(void){

  // Initial Arduino hardware interface connect with CC3000.
  CC3000_Init();

  // Initial callback functions to wlan api.
  wlan_init( Spider_AsyncCallback, SendFirmwarePatch, SendDriverPatch, SendBootloaderPatch, 
             ReadWlanInterruptPin, WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);

  // Starting CC3000
  wlan_start(0);

  // Set CC3000 event masking, right now without ping report.
  wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

  //Initial success.
  HW_Initialed = 1;
  return 0;
}
/*------------------------------------------------------------------------
  Spider_close
  
  Initial Spider L3, 
  
  return  0, Initial success.
  
-----------------------------------------------------------------------*/
int Spider_close(void){
  
  wlan_stop();
  HW_Initialed = 0;
  return 0;
}
/*------------------------------------------------------------------------
  
  Spider_SmartConfig
  Use Smart config function set Spider connect to a exist AP.
  
  return  0, Connect success.
-----------------------------------------------------------------------*/
int Spider_SmartConfig(void){
    SmartConfigFinished = 0;
    SpiderConnected = 0;
    SpiderDHCP = 0;
    SpiderCanShutDown = 0;

    // Reset all the previous configuration
    wlan_ioctl_set_connection_policy(0, 0, 0);
    wlan_ioctl_del_profile(255);

    //Wait until Spider is disconnected
    while (SpiderConnected == 1){
        delay(1);
        hci_unsolicited_event_handler();
    }

    // create new entry for AES encryption key
    nvmem_create_entry(NVMEM_AES128_KEY_FILEID, AES128_KEY_SIZE);

    // write AES key to NVMEM
    aes_write_key((unsigned char *)(&SmartConfig_key[0]));

    // Start blinking LED1 during Smart Configuration process
    digitalWrite(13, HIGH);
    wlan_smart_config_set_prefix((char*)SmartConfig_prefix);
    digitalWrite(13, LOW);

    // Start the SmartConfig start process
    if(wlan_smart_config_start(0) != 0){
      return -1;
    }

    // Wait for Smart config to finish
    while (SmartConfigFinished == 0){
        digitalWrite(13, HIGH);
        delay(500);
        digitalWrite(13, LOW);
        delay(500);
    }
    digitalWrite(13, LOW);


    // Decrypt configuration information and add profile
    //wlan_smart_config_process();

#if 1
    // Configure to connect automatically to the AP retrieved in the
    // Smart config process
    wlan_ioctl_set_connection_policy(0, 0, 1);

    // reset the CC3000
    wlan_stop();

    delay(100);

    wlan_start(0);

    // Set CC3000 event masking, right now without ping report.
    wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

#endif

    return 0;
}
/*------------------------------------------------------------------------
  
  Spider_SmartConfig_AckFinish
  Acknowledge smart config finished message to smart config APP.
  
  return  0, Ack success.
  return -1, mdns functions failed, ack failed.
-----------------------------------------------------------------------*/
int Spider_SmartConfig_AckFinish(char* ack_msg){
    int i;
    for(i = 0; i < 3; i++){
        digitalWrite(13, HIGH);
        delay(10);
        digitalWrite(13, LOW);
        //Ack mDNS package to SmartConfig phone to finished connection process.
        if(mdnsAdvertiser(1, (char*)ack_msg, strlen(ack_msg)) != 0){
          return -1;
        }
    }
    return 0;
}

/*------------------------------------------------------------------------

  Spider_AutoConnect
  Use SmartConfig profile connect to a exist AP.

  return  0, Connect success.
  return -1, Hardware uninitialized.
-----------------------------------------------------------------------*/
int Spider_AutoConnect(void){
    long ret;

    if(HW_Initialed != 1) return -1;

    // Check and wait until CC3000 disconnect from current network.
    while (SpiderConnected == 1) {
        delay(10);
    }

    ret = wlan_ioctl_set_connection_policy(0, 0, 1);

    // Check connection failed.
    if(ret != 0){
      return -2;
    }

    return 0;
}

/*------------------------------------------------------------------------

  Spider_Disconnect
  Dissconnect SpiderL3 from a exist AP.

  return  0, Connect success.
  return -1, Hardware uninitialized.
-----------------------------------------------------------------------*/
int Spider_Disconnect(void){
    long ret;

    if(HW_Initialed != 1) return -1;

    ret = wlan_disconnect();

    // Check connection failed.
    if(ret != 0){
      return -2;
    }

    return 0;
}

/*------------------------------------------------------------------------
  
  Spider_Connect
  Setting Spider connect to a exist AP. 

  unsigned long sec_mode, security mode, 
                          OPEN = 0, WEP = 1, WPA = 2, WPA2 = 3,
                          Reference with header file wlan.h.
  char* tar_ssid        , target AP's SSID.
  char* tar_password    , target AP's Password, 
                          leaving null with OPEN security.

  
  return  0, Connect success.
  return -1, Hardware uninitialized.
  return -2, Connection failed.
-----------------------------------------------------------------------*/
int Spider_Connect(unsigned long sec_mode, char* tar_ssid, char* tar_password){
  long ret;

  if(HW_Initialed != 1) return -1;

  // Disable auto connection function.
  wlan_ioctl_set_connection_policy(0, 0, 0);

  // Check and wait until CC3000 disconnect from current network.
  while (SpiderConnected == 1) {
    delay(100);
  }
  
  // Connect to AP
  ret = wlan_connect(sec_mode, tar_ssid, strlen(tar_ssid), 0, (unsigned char*)tar_password, strlen(tar_password));

  // Check connection failed.
  if(ret != 0){
    return -2;
  }

  // Wait connect flag.
  while(SpiderConnected != 1){
    delay(100);
  }

  return 0;
}
/*------------------------------------------------------------------------
  
  Spider_CheckDHCP
  Check CC3000 get ip address from DHCP server or not. 

  return  0, Get address.
  return -1, waiting address.
-----------------------------------------------------------------------*/
int Spider_CheckDHCP(void){
  if(SpiderDHCP == 1){
    return 0;
  } 
  else{
    return -1;
  }
}

/*------------------------------------------------------------------------
  
  Spider_CheckConnected
  Check CC3000 get ip address from DHCP server or not. 

  return  0, Get address.
  return -1, waiting address.
-----------------------------------------------------------------------*/
int Spider_CheckConnected(void){
  if(SpiderConnected == 1){
    return 0;
  } 
  else{
    return -1;
  }
}