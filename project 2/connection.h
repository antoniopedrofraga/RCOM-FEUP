#pragma once

#include "utilities.h"
#include "url.h"


typedef struct Connection {
	int fd;
	char * ip;
	int port;
} connection;

int get_ip(connection * connection, url* url, int debug_mode);

int connect_host(connection * connection, url * url, int debug_mode);

int connect_to(char * ip, int port, int debug_mode);

int log_in_host(connection * connection, url * url, int debug_mode);

int pasv_host(connection * connectionA, url * url, int debug_mode, connection * connectionB);

int get_pasv_from_host(int connection_fd, char* ip, int * port, int debug_mode);

int def_path(connection * connectionA,char * path, int debug_mode);

int download_from_host(connection * connectionB, char* path, int debug_mode);

int disconnect_host(connection * connectionA, url * url, int debug_mode);

int send_to_host(int connection_fd, const char* msg);

int read_from_host(int connection_fd, char* msg, int debug_mode, char * code);
