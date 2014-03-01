#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__
/*****************************************************************************
*
*  WebServer.h.
*  
*****************************************************************************/
#include "Spider_L3.h"

#ifdef  __cplusplus
extern "C" {
#endif

extern int WebServer_begin(unsigned short *port);
extern int WebServer_close(int server_socket);
extern int WebServer_wait_connect(int socket);
extern int WebServer_get_request(int socket, char* req_buf, unsigned long buf_len);
extern int WebServer_put_response(int socket, char* resp_buf, unsigned long buf_len);
extern int WebServer_put_badrequest(int socket);
extern int WebServer_close_connect(int socket);


#ifdef  __cplusplus
}
#endif

#endif //__WEBSERVER_H__