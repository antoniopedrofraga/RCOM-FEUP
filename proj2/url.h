#pragma once

typedef struct Url {
	char * user; 
	char * password;
	char * host;
	char * ip; 
	char * path; 
	char * filename; 
	int port;
} url;


int init_url(url * url, char * url_str, int debug_mode);
int get_ip(url* url, int debug_mode);