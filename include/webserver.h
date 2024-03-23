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

#define MAX_SCK_REQ_QUEUED 10
#define BUFFER_SZ 2048

typedef struct webserver {
    int sck;
} webserver_t;

webserver_t Init_Webserver (const uint16_t port);

void* Listening_Webserver (void* args);

void _Handle_Client_Webserver (int connfd, char* buffer);
