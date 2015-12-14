#include "socket.h"

int connect_host(sckt * sckt, url * url, int debug_mode) {

	bzero((char*)&(sckt->server_addr),sizeof((sckt->server_addr)));
	sckt->server_addr.sin_family = AF_INET;
	sckt->server_addr.sin_addr.s_addr = inet_addr(url->ip);	
	sckt->server_addr.sin_port = htons(url->port);

	debug_sub_msg(debug_mode, "Creating a socket...");

	if ((sckt->fd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("\t->Error, could not execute socket()");
        	return ERROR;
    }

    debug_sub_msg(debug_mode, "Socket created.");

    debug_sub_msg(debug_mode, "Connecting...");

    if (connect(sckt->fd, (struct sockaddr *)&(sckt->server_addr), sizeof(sckt->server_addr)) < 0){
        perror("\t->Error, could not execute connect()");
		return ERROR;
	}
	debug_sub_msg(debug_mode, "Connected!");

	return OK;
}

int log_in_host(sckt * sckt, url * url, int debug_mode) {

	char * user = malloc(sizeof(url->user) + 5 * sizeof(char));
	sprintf(user, "user %s\r\n", url->user);

	debug_sub_msg(debug_mode, "Sending user to host...");

	if (send_to_host(sckt->fd, user) == ERROR) {
		printf("\t->Error sending a message to host.");
		return ERROR;
	} 

	debug_sub_msg(debug_mode, "User sent!");

	debug_sub_msg(debug_mode, "Receiving message from host...");

	read_from_host(sckt->fd, user);

	debug_sub_msg(debug_mode, "Message received!");


	char * password = malloc(sizeof(url->password) + 5 * sizeof(char));
	sprintf(password, "pass %s\r\n", url->password);

	debug_sub_msg(debug_mode, "Sending password to host...");

	if (send_to_host(sckt->fd, password) == ERROR) {
		printf("\t->Error sending a message to host.");
		return ERROR;
	} 

	debug_sub_msg(debug_mode, "Password sent!");

	debug_sub_msg(debug_mode, "Receiving message from host...");

	read_from_host(sckt->fd, password);

	debug_sub_msg(debug_mode, "Message received!");


	return OK;
}


int pasv_host(sckt * sckt, url * url, int debug_mode) {

	char * pasv = malloc(7 * sizeof(char));
	sprintf(pasv, "pasv \r\n");

	debug_sub_msg(debug_mode, "Sending password to host...");

	if (send_to_host(sckt->fd, pasv) == ERROR) {
		printf("\t->Error sending a message to host.");
		return ERROR;
	} 

	debug_sub_msg(debug_mode, "Password sent!");

	debug_sub_msg(debug_mode, "Receiving message from host...");

	read_from_host(sckt->fd, pasv);

	debug_sub_msg(debug_mode, "Message received!");

	return OK;
}



int send_to_host(int sckt_fd, const char* msg) {
	
	int written_bytes = 0;

	written_bytes = write(sckt_fd, msg, strlen(msg));

	int return_value = (written_bytes == strlen(msg)) ? OK : ERROR; 

	return return_value;

}


int read_from_host(int sckt_fd, char* msg) {

	FILE* fp = fdopen(sckt_fd, "r");
	int size = sizeof(msg);
	printf("\t->Message: ");
	do {
		memset(msg, 0, size);
		msg = fgets(msg, size, fp);
		printf("%s", msg);
	} while (!('1' <= msg[0] && msg[0] <= '5') || msg[3] != ' ');

	printf("\n");
	return OK;
}

