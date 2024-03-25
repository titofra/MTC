#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <libwebsockets.h>
#include "mutex.h"

#define UNUSED(expr) do { (void) (expr); } while (0)

typedef struct webserver {
    struct lws_context* context;
    struct lws_protocols protocols [3];
} webserver_t;

webserver_t* Init_Webserver (int port);

void* Listening_Webserver (void* args);

void Send_Webserver (webserver_t* ws, const char* data, size_t len);

void Free_Webserver (webserver_t* ws);

#endif  // WEBSERVER_H
