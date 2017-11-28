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
#include <sys/stat.h>
#include <fcntl.h>

#define SERVER_PORT   6000              // PORT
#define SERVER_ADDR   "192.168.28.96"   // IP
#define STR_SIZE      256               // Max string size
#define CODE_LENGTH   3                 //SMTP reply code length
#define COMMAND_PORT  21

typedef struct {
        char*   host;       // host or host:port
        char*   path;       // path (relative paths may omit leading slash)
        char*   file_name;
        char*   ip;
} url_t;

typedef struct {
        char* username;
        char* password;
} user_t;

extern url_t* url;

extern user_t* user;
