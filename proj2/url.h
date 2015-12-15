#pragma once

typedef struct Url {
	char * user; 
	char * password;
	char * host;
	char * path; 
	char * filename;
} url;


int init_url(url * url, char * url_str, int debug_mode);