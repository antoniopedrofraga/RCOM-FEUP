#include "utilities.h"
#include "url.h"


int init_url(url * url, char * url_str, int debug_mode) {

	debug_sub_msg(debug_mode, "Getting url strings...");

	char str_beg[6];
	strncpy(str_beg, url_str, 6);
	
	/*if (strcmp(str_beg, "ftp://") != 0) {
		printf("\nError! Please start your url by 'ftp://'...\n");
		return ERROR;
	}*/

	char ** sub_str = malloc(5 * sizeof(char*));

	int size =  strlen(url_str) - 6;
	sub_str[0] = malloc(size);
	memcpy(sub_str[0], url_str + 6, size);
	sub_str[0][size] = '\0';


	url->user = malloc(strlen(sub_str[0]));
	memcpy(url->user, sub_str[0], strlen(sub_str[0]));
	strtok(url->user, ":");

	if (strlen(url->user) == strlen(sub_str[0])) {
		printf("\nError!!! Please declare an user...\n");
		return ERROR;
	}

	size = strlen(sub_str[0]) - strlen(url->user) - 1;
	sub_str[1] = malloc(size);
	memcpy(sub_str[1], sub_str[0] + strlen(url->user) + 1, size);
	sub_str[1][size] = '\0';

	url->password = malloc(strlen(sub_str[1]));
	memcpy(url->password, sub_str[1], strlen(sub_str[1]));
	strtok(url->password, "@");

	if (strlen(url->password) == strlen(sub_str[1])) {
		printf("\nError!!! Please declare a password...\n");
		return ERROR;
	}

	size = strlen(sub_str[1]) - strlen(url->password) - 1;
	sub_str[2] = malloc(size);
	memcpy(sub_str[2], sub_str[1] + strlen(url->password) + 1, size);
	sub_str[2][size] = '\0';

	url->host = malloc(strlen(sub_str[2]));
	memcpy(url->host, sub_str[2], strlen(sub_str[2]));
	strtok(url->host, "/");

	if (strlen(url->host) == strlen(sub_str[2])) {
		printf("\nError! Please declare a host...\n");
		return ERROR;
	}

	size = strlen(sub_str[2]) - strlen(url->host) - 1;
	sub_str[3] = malloc(size);
	memcpy(sub_str[3], sub_str[2] + strlen(url->host) + 1, size);
	sub_str[3][size] = '\0';

	url->path = malloc(strlen(sub_str[3]));
	size = strlen(sub_str[3]);
	memcpy(url->path, sub_str[3], size);
	url->path[size - 1] = '\0';

	if (!strlen(url->path)) {
		printf("\nError! Please declare a path...\n");
		return ERROR;
	}

	debug_sub_msg(debug_mode, "Completed!");
	
	return OK;
}


