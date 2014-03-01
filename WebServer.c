/*****************************************************************************
*
*  WebServer.c.
*  
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include "WebServer.h"

int WebServer_begin(unsigned short *port){
    int httpd = 0;
    sockaddr_in name;

    httpd = socket(AF_INET, SOCK_STREAM, 0);
    if(httpd < 0){
        return -1;
    }

    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(*port);
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

    long timeout = 500;
    Connect_stat = setsockopt(client_sock, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &timeout, sizeof(timeout));
    if(Connect_stat < 0){
        closesocket(client_sock);
        client_sock = -1;
        return -1;
    }

    return client_sock;
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

int WebServer_close_connect(int socket){
    return closesocket(socket);
}