#ifndef __WEBCLIENT_H__
#define __WEBCLIENT_H__
/*****************************************************************************
*
*  WebClient.h.
*  
*****************************************************************************/
#include "Spider_L3.h"

//
// Web server port
//
#define DEFAULT_WEB_PORT	80

//
// Web server reciving timeout
//
#define WEB_CLIENT_RECV_TO	2000

//
// Web client debug print
//
#define WEB_CLIENT_DBG_EN	0

enum{
	OK 						=  0,
	ERROR 					= -1,
	HOST_STR_FAIL 			= -2,
	HOST_IP_FAIL 			= -3,
	GET_SOCKET_FAIL 		= -4,
	SET_SOCKET_TO_FAIL		= -5,
	WEB_SERVER_CONN_FAIL 	= -6,
	WEB_SERVER_SEND_FAIL	= -7,
	WEB_SERVER_RECV_FAIL	= -8
};

typedef struct{
	long		host_socket;
	sockaddr	host_addr;
}Host_Info;

#ifdef  __cplusplus
extern "C" {
#endif
extern int WebClient_Get_HostIP(char* url_link, unsigned char* ret_IP0, unsigned char* ret_IP1, unsigned char* ret_IP2, unsigned char* ret_IP3);

extern int WebClient_Begin_URL(Host_Info *info, char* url_link);

extern int WebClient_Begin_IP(Host_Info *info, unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned char ip4, unsigned short port);

extern int WebClient_Connect(Host_Info *info);
extern int WebClient_SendRequest(Host_Info *info, char* url_link);
extern int WebClient_DataAvailable(Host_Info *info);
extern int WebClient_RecvData(Host_Info *info, char* ret_buf, int buf_size);
extern int WebClient_Get(Host_Info *info, char* url_link, char* ret_buf, int buf_size);
extern int WebClient_Close(Host_Info *info);

#ifdef  __cplusplus
}
#endif

#endif //__WEBCLIENT_H__