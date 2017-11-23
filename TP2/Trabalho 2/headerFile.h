#pragma once

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>

#define SERVER_PORT 6000              // PORT
#define SERVER_ADDR "192.168.28.96"   // IP

typedef struct {
        User_t  user_info;  // username and password information
        char*   host;       // host or host:port
        char*   path;       // path (relative paths may omit leading slash)
        char*   file_name;
        char*   ip;
} URL_t;

typedef struct {
        char* username;
        char* password;
} User_t;
