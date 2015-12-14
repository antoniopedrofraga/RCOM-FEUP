#include "utilities.h"
#include "url.h"
#include "socket.h"


int main (int argc, char** argv) {
	
	if (argc != 3) {
		printf("\nUsage Error!\n");
		printf("\n\nUsage:\n");
		printf("| name |\t\t url\t\t\t| debug mode |\n");
		printf(" ./app ftp://[<user>:<password>@]<host>/<url-path> <ON/OFF>\n\n");
		return ERROR;	
	}

	if (strcmp(argv[2], "ON") != 0 && strcmp(argv[2], "OFF") != 0) {
		printf("Error!\n");
		printf("\nPlease give a valid debug mode: ON/OFF\n");
		return ERROR;
	}

	int debug_mode = strcmp(argv[2], "ON") == 0 ? 1 : 0;


	debug_msg(debug_mode, "Initializing a url struct.");

	url *new_url = malloc(sizeof(url));
	if (init_url(new_url, argv[1], debug_mode) == ERROR)
		return ERROR;

	debug_msg(debug_mode, "Url struct initialized.\n");
	debug_msg(debug_mode, "Getting ip by host name.");

	if (get_ip(new_url, debug_mode) == ERROR)
		return ERROR;

	debug_msg(debug_mode, "Host is valid, and it returned a valid ip.\n");

	char * host_info = malloc(30 * sizeof(char));
	strcpy(host_info, "Connecting to ");
	strcat(host_info, new_url->ip);
	strcat(host_info, "...");

	debug_msg(debug_mode, host_info);

	sckt * new_sckt = malloc(sizeof(sckt));

	new_sckt->fd = connect_to(new_url->ip, new_url->port, debug_mode);

	if (new_sckt->fd == ERROR) 
		return ERROR;

	debug_msg(debug_mode, "Connection was successfull.\n");

	debug_msg(debug_mode, "Logging in.");

	if (log_in_host(new_sckt, new_url, debug_mode) == ERROR) 
		return ERROR;

	debug_msg(debug_mode, "Logged in messages were sent.\n");

	debug_msg(debug_mode, "Entering passive mode...");

	if (pasv_host(new_sckt, new_url, debug_mode) == ERROR) 
		return ERROR;

	debug_msg(debug_mode, "Completed");

	char * data_host_info = malloc(30 * sizeof(char));
	strcpy(data_host_info, "Connecting to ");
	strcat(data_host_info, new_sckt->server_ip);
	strcat(data_host_info, "...");

	debug_msg(debug_mode, data_host_info);

	debug_msg(debug_mode, "Connected!");




	debug_msg(debug_mode, "Disconnecting...");

	if (disconnect_host(new_sckt, debug_mode) == ERROR) 
		return ERROR;

	debug_msg(debug_mode, "Diconnected.");

	return OK;
}
