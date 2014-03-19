/*****************************************************************************
*
*  WebServer.c - Web server function for Spider_L3
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
#include "WebServer.h"

int WebServer_begin(unsigned short port){
    int httpd = 0;
    sockaddr_in name;

    httpd = socket(AF_INET, SOCK_STREAM, 0);
    if(httpd < 0){
        return -1;
    }

    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(0);

    if (bind(httpd, (sockaddr *)&name, sizeof(name)) < 0){
        closesocket(httpd);
        return -2;
    }

    if (listen(httpd, 0) < 0){
        closesocket(httpd);
        return -3;
    }

    return(httpd);
}

int WebServer_close(int server_socket){
    return closesocket(server_socket);
}



int WebServer_wait_connect(int socket){
    long Connect_stat = -1;
    int client_sock = -1;
    sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);

    client_sock = accept(socket, (sockaddr *)&client_name, &client_name_len);
    if(client_sock < 0){
        return -1;
    }

    long timeout = 100;
    Connect_stat = setsockopt(client_sock, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &timeout, sizeof(timeout));
    if(Connect_stat < 0){
        closesocket(client_sock);
        client_sock = -1;
        return -1;
    }

    return client_sock;
}

int WebServer_request_available(int socket){
    timeval timeout;
    fd_set fd_read;

    memset(&fd_read, 0, sizeof(fd_read));
    FD_SET(socket, &fd_read);

    timeout.tv_sec = 0;
    timeout.tv_usec = 5000; // 5 millisec

    return select(socket, &fd_read, NULL, NULL, &timeout);
}

int WebServer_get_request(int socket, char* req_buf, unsigned long buf_len){
    int             Connect_stat = -1;

    // Receiving http get result from host server.
    Connect_stat = recv(socket, req_buf, buf_len, 0);
    if(Connect_stat < 0){
        return -1;
    }

    return Connect_stat;
}

PROGMEM const char GET_Str[] =  "GET ";
PROGMEM const char POST_Str[] = "POST ";


#if 1
int WebServer_process_request(int socket, char* filename, unsigned char name_buf_size, char* CGI_param, unsigned char CGI_buf_size){

    unsigned long tmr = 0;
    char data_pool[100];
    int ret = 0;

    int get_method = -1;

    char* str1 = 0;
    char* str2 = 0;
    unsigned long cal_1 = 0;

    // Pre-load http request from Spider L3
    memset(data_pool, 0, sizeof(data_pool));
    tmr = millis() + WEBSERVER_REQUEST_TIMEOUT;
    ret = 0;
    while(millis() < tmr){
        ret = WebServer_get_request(socket, data_pool, sizeof(data_pool));
        if(ret >= 0) break;
        if(ret < 0) return WEBSERVER_FAIL;
    }

    while(1){
        // Find method
        str1 = strstr_P(data_pool, GET_Str);
        if(str1 != 0){
            get_method = WEBSERVER_METHOD_GET;
            str1 += 4;
            break;
        }
        else{
            str1 = strstr_P(data_pool, POST_Str);
            if(str1 != 0){
                get_method = WEBSERVER_METHOD_POST;
                str1 += 5;
                break;
            }
            else{
                // Do Streaming read.
                memset(data_pool, 0, sizeof(data_pool));
                tmr = millis() + WEBSERVER_REQUEST_TIMEOUT;
                ret = 0;
                while(millis() < tmr){
                    ret = WebServer_get_request(socket, data_pool, sizeof(data_pool));
                    if(ret >= 0) break;
                    if(ret < 0) return WEBSERVER_FAIL;
                }

                // Cannot get file name
                if(ret == 0) return get_method;

                str1 = data_pool;
            }
            /*
            else{
                //Unsupported method, read rest data...
                tmr = millis() + WEBSERVER_REQUEST_TIMEOUT;
                ret = 0;
                while(millis() < tmr){
                    ret = WebServer_get_request(socket, data_pool, sizeof(data_pool));
                    if(ret < 0) return WEBSERVER_FAIL;
                }
                return WEBSERVER_FAIL;
            }
            */

        }
    }

    // Get filename
    str2 = data_pool + sizeof(data_pool); // Limit of pool
    cal_1 = 0;
    while(1){
        if(str1 < str2){
            // Check stop char space or CGI char ?
            if(((*str1 == ' ') || (*str1 == '?')) && (cal_1 < name_buf_size)){
                break;
            }
            else{
                filename[cal_1] = *str1;
                str1++;
                cal_1++;
            }
        }
        else{
            // Do Streaming read.
            memset(data_pool, 0, sizeof(data_pool));
            tmr = millis() + WEBSERVER_REQUEST_TIMEOUT;
            ret = 0;
            while(millis() < tmr){
                ret = WebServer_get_request(socket, data_pool, sizeof(data_pool));
                if(ret >= 0) break;
                if(ret < 0) return WEBSERVER_FAIL;
            }

            // Cannot get file name
            if(ret == 0) return get_method;

            str1 = data_pool;
        }
    }

    // Find CGI parameter start position
    str2 = data_pool + sizeof(data_pool); // Limit of pool
    if(get_method == WEBSERVER_METHOD_GET){
        while(1){
            if(str1 < str2){
                if(*str1 == '?'){
                    str1 += 1;
                    break;
                }
                else{
                    str1++;
                }
            }
            else{
                // Do Streaming read.
                memset(data_pool, 0, sizeof(data_pool));
                tmr = millis() + WEBSERVER_REQUEST_TIMEOUT;
                ret = 0;
                while(millis() < tmr){
                    ret = WebServer_get_request(socket, data_pool, sizeof(data_pool));
                    if(ret > 0) break;
                    if(ret < 0) return WEBSERVER_FAIL;
                }
                // Cannot get CGI parameter start position
                if(ret == 0) return get_method;

                str1 = data_pool;
            }
        }
    }
    else if(get_method == WEBSERVER_METHOD_POST){
        while(1){
            if(str1 < (str2 - 4)){
                if((str1[0] == '\r') && (str1[1] == '\n') && (str1[2] == '\r') && (str1[3] == '\n')){
                    str1 += 4;
                    break;
                }
                else{
                    str1++;
                }
            }
            else{
                // Do Streaming read.
                strncpy(data_pool, str1, 4);
                memset(&data_pool[4], 0, (sizeof(data_pool) - 4));
                tmr = millis() + WEBSERVER_REQUEST_TIMEOUT;

                ret = 0;
                while(millis() < tmr){
                    ret = WebServer_get_request(socket, &data_pool[4], (sizeof(data_pool) - 4));
                    if(ret > 0) break;
                    if(ret < 0) return WEBSERVER_FAIL;
                }

                // Cannot get CGI parameter start position
                if(ret == 0) return get_method;

                str1 = data_pool + 1;
            }
        }
    }

    // Get CGI parameter
    str2 = data_pool + sizeof(data_pool); // Limit of pool
    cal_1 = 0;
    while(1){
        if(str1 < str2){
            if((*str1 != 0) && (*str1 != ' ') && (cal_1 < CGI_buf_size)){
                CGI_param[cal_1] = *str1;
                str1++;
                cal_1++;
            }
            else{
                break;
            }
        }
        else{
            // Do Streaming read.
            memset(data_pool, 0, sizeof(data_pool));
            tmr = millis() + WEBSERVER_REQUEST_TIMEOUT;
            ret = 0;
            while(millis() < tmr){
                ret = WebServer_get_request(socket, data_pool, sizeof(data_pool));
                if(ret > 0) break;
                if(ret < 0) return WEBSERVER_FAIL;
            }

            // Cannot get CGI parameter
            if(ret == 0) return get_method;

            str1 = data_pool;
        }
    }

    // Read rest data...
    tmr = millis() + WEBSERVER_REQUEST_TIMEOUT;
    ret = 0;
    while(millis() < tmr){
        ret = WebServer_get_request(socket, data_pool, sizeof(data_pool));
        if(ret <= 0) break;
    }
    return get_method;
}
#endif
int WebServer_put_response(int socket, char* resp_buf, unsigned long buf_len){

    int             Connect_stat = -1;

    Connect_stat = send(socket, resp_buf, buf_len, 0);
    if(Connect_stat < 0){
        return -1;
    }
    return Connect_stat;
}

int WebServer_put_badrequest(int socket){
    char req_pool[50];

    memset(req_pool, 0, sizeof(req_pool));
    strncpy_P(req_pool, PSTR("HTTP/1.0 400 Bad Request\r\n"), sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    memset(req_pool, 0, sizeof(req_pool));
    strncpy_P(req_pool, PSTR("Content-Type: text/html\r\n"), sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    memset(req_pool, 0, sizeof(req_pool));
    strncpy_P(req_pool, PSTR("Connection: close\r\n\r\n"), sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    memset(req_pool, 0, sizeof(req_pool));
    strncpy_P(req_pool, PSTR("<P>BAD REQUEST\r\n"), sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    return 0;
}

int WebServer_put_notfound(int socket){
    char req_pool[50];

    memset(req_pool, 0, sizeof(req_pool));
    strncpy_P(req_pool, PSTR("HTTP/1.0 404 Not Found\r\n"), sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    memset(req_pool, 0, sizeof(req_pool));
    strncpy_P(req_pool, PSTR("Content-Type: text/html\r\n"), sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    memset(req_pool, 0, sizeof(req_pool));
    strncpy_P(req_pool, PSTR("Connection: close\r\n\r\n"), sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    return 0;
}

int WebServer_close_connect(int socket){
    return closesocket(socket);
}