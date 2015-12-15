#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#include <libgen.h>


#define ERROR -1
#define OK 0

void debug_msg (int debug_mode, char * msg);
void debug_sub_msg (int debug_mode, char * msg);