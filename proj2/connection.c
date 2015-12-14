#include "connection.h"

int connect_to(char * ip, int port, int debug_mode) {

	struct sockaddr_in server_addr;
	bzero((char*)&(server_addr),sizeof((server_addr)));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	
	server_addr.sin_port = htons(port);

	debug_sub_msg(debug_mode, "Creating a socket...");

	int fd;

	if ((fd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("\t->Error, could not execute socket()");
        	return ERROR;
    }

    debug_sub_msg(debug_mode, "Socket created.");

    debug_sub_msg(debug_mode, "Connecting...");

    if (connect(fd, (struct sockaddr *)&(server_addr), sizeof(server_addr)) < 0){
        perror("\t->Error, could not execute connect()");
		return ERROR;
	}

	debug_sub_msg(debug_mode, "Connected!");

	char * msg = malloc(5 * sizeof(char));

	debug_sub_msg(debug_mode, "Receiving response message...");

	read_from_host(fd, msg, debug_mode);

	debug_sub_msg(debug_mode, "Message received!");


	return fd;
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

	read_from_host(sckt->fd, user, debug_mode);

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

	read_from_host(sckt->fd, password, debug_mode);

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

	debug_sub_msg(debug_mode, "Interpreting passive message from host...");

	char * ip = malloc(50 * sizeof(char));

	int port;

	if( get_pasv_from_host(sckt->fd, ip, &port, debug_mode) < 0 ) {
		printf("\t->Error interpreting passive message.\n");
		return ERROR;
	}

	char * ip_info = malloc(1024 * sizeof(char));

	strcpy(ip_info, "Interpreted IP: \0");
	strcat(ip_info, ip);

	debug_sub_msg(debug_mode, ip_info);

	char * port_info = malloc(1024 * sizeof(char));

	if (sprintf(port_info, "Interpreted Port: %d", port) < 0)  {
		printf("\t->Error printing port to string.\n");
		return ERROR;
	}

	sckt->server_ip = ip;
	sckt->server_port = port;

	debug_sub_msg(debug_mode, port_info);

	debug_sub_msg(debug_mode, "Completed!");

	return OK;
}



int disconnect_host(sckt * sckt, int debug_mode) {
	
	char * quit = malloc(6 * sizeof(char));
	
	sprintf(quit, "quit\r\n");

	debug_sub_msg(debug_mode, "Sending 'quit' command to host...");
	
	if (send_to_host(sckt->fd, quit) == ERROR) {
		printf("\t->Error sending a message to host.");
		return ERROR;
	}

	debug_sub_msg(debug_mode, "Closing socket...");

	if (sckt->fd)
		close(sckt->fd);

	return OK;
}



int send_to_host(int sckt_fd, const char* msg) {
	
	int written_bytes = 0;

	written_bytes = write(sckt_fd, msg, strlen(msg));

	int return_value = (written_bytes == strlen(msg)) ? OK : ERROR; 

	return return_value;

}


int read_from_host(int sckt_fd, char* msg, int debug_mode) {

	FILE* fp = fdopen(sckt_fd, "r");
	int size = 4;
	
	if(debug_mode)
		printf("\t->Message Code: ");

	do {
		memset(msg, 0, size);
		msg = fgets(msg, size, fp);
		
		if(debug_mode)
			printf("%s", msg);

	} while (!('1' <= msg[0] && msg[0] <= '5'));

	if(debug_mode)
		printf("\n");

	free(msg);

	return OK;
}



int get_pasv_from_host(int sckt_fd, char* ip_str, int * port,int debug_mode) {

	FILE* fp = fdopen(sckt_fd, "r");
	int size = 1024;
	char * msg = malloc(size * sizeof(char));
	
	if(debug_mode)
		printf("\t->Passive Message: ");

	do {
		memset(msg, 0, size);
		msg = fgets(msg, size, fp);
		
		if(debug_mode)
			printf("%s", msg);

	} while (!('1' <= msg[0] && msg[0] <= '5'));

	int ip[4];
	int port_arr[2];

	if ((sscanf(msg, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", 
		&ip[0], &ip[1], &ip[2], &ip[3], &port_arr[0], &port_arr[1])) < 0)
		return ERROR;

	if (sprintf(ip_str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]) < 0)
		return ERROR;

	*port = 256 * port_arr[0] + port_arr[1];

	free(msg);

	return OK;
}

