#pragma once

#include "utilities.h"
#include "url.h"


typedef struct Socket {
	int	fd;
	int data_fd;
	char * server_ip;
	int server_port;
} sckt;

int connect_host(sckt * sckt, url * url, int debug_mode);

int connect_to(char * ip, int port, int debug_mode);

int log_in_host(sckt * sckt, url * url, int debug_mode);

int pasv_host(sckt * sckt, url * url, int debug_mode);

int get_pasv_from_host(int sckt_fd, char* ip, int * port, int debug_mode);

int disconnect_host(sckt * sckt, int debug_mode);

int send_to_host(int sckt_fd, const char* msg);

int read_from_host(int sckt_fd, char* msg, int debug_mode);