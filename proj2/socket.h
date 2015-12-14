#pragma once

#include "utilities.h"
#include "url.h"


typedef struct Socket {
	int	fd;
	struct	sockaddr_in server_addr;
} sckt;

int connect_host(sckt * sckt, url * url, int debug_mode);

int log_in_host(sckt * sckt, url * url, int debug_mode);

int pasv_host(sckt * sckt, url * url, int debug_mode);

int send_to_host(int sckt_fd, const char* msg);

int read_from_host(int sckt_fd, char* msg);