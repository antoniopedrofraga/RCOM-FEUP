#include "utilities.h"
#include "url.h"
#include "connection.h"


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

	connection * connectionA = malloc(sizeof(connection));

	if (get_ip(connectionA, new_url, debug_mode) == ERROR)
		return ERROR;

	debug_msg(debug_mode, "Host is valid, and it returned a valid ip.\n");

	char * host_info = malloc(30 * sizeof(char));
	strcpy(host_info, "Connecting to A '");
	strcat(host_info, connectionA->ip);
	strcat(host_info, "' through port ");
	char portA_str[15];
	sprintf(portA_str, "%d", connectionA->port);
	strcat(host_info, portA_str);
	strcat(host_info, "...");

	debug_msg(debug_mode, host_info);

	connectionA->fd = connect_to(connectionA->ip, connectionA->port, debug_mode);

	if (connectionA->fd == ERROR) 
		return ERROR;

	debug_msg(debug_mode, "Connection was successfull.\n");

	debug_msg(debug_mode, "Logging in.");

	if (log_in_host(connectionA, new_url, debug_mode) == ERROR) 
		return ERROR;

	debug_msg(debug_mode, "Logged in messages were sent.\n");

	debug_msg(debug_mode, "Entering passive mode...");

	connection * connectionB = malloc(sizeof(connection));

	if (pasv_host(connectionA, new_url, debug_mode, connectionB) == ERROR) 
		return ERROR;

	debug_msg(debug_mode, "Completed!\n");

	char * data_host_info = malloc(30 * sizeof(char));
	strcpy(data_host_info, "Connecting to B '");
	strcat(data_host_info, connectionA->ip);
	strcat(data_host_info, "' through port ");
	char portB_str[15];
	sprintf(portB_str, "%d", connectionB->port);
	strcat(data_host_info, portB_str);
	strcat(data_host_info, "...");

	debug_msg(debug_mode, data_host_info);

	connectionB->fd = connect_to(connectionB->ip, connectionB->port, debug_mode);

	if (connectionB->fd == ERROR) 
		return ERROR;

	debug_msg(debug_mode, "Connected!\n");

	debug_msg(debug_mode, "Sending path...");

	if (def_path(connectionA, new_url->path, debug_mode) == ERROR)
		return ERROR;

	debug_msg(debug_mode, "Path was sent!\n");

	debug_msg(1, "Downloading file...");

	if (download_from_host(connectionB, new_url->path, debug_mode) == ERROR)
		return ERROR;

	debug_msg(1, "Download completed...\n");

	debug_msg(debug_mode, "Disconnecting...");

	if (disconnect_host(connectionA, new_url, debug_mode) == ERROR) 
		return ERROR;

	debug_msg(debug_mode, "Diconnected.\n");

	return OK;
}
