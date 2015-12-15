#include "connection.h"


int get_ip(connection * connection, url* url, int debug_mode) {
	struct hostent* h;

	debug_sub_msg(debug_mode, "Getting host ip by name...");

	if ((h = gethostbyname(url->host)) == NULL) {
		herror("Error, could not execute gethostbyname()");
		return ERROR;
	}

	debug_sub_msg(debug_mode, "Completed!");

	char* ip = inet_ntoa(*((struct in_addr *) h->h_addr));

	connection->ip = malloc(strlen(ip));
	strcpy(connection->ip, ip);

	connection->port = 21;

	return OK;
}


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
        	exit(ERROR);
    }

    debug_sub_msg(debug_mode, "Socket created.");

    debug_sub_msg(debug_mode, "Connecting...");

    if (connect(fd, (struct sockaddr *)&(server_addr), sizeof(server_addr)) < 0){
        perror("\t->Error, could not execute connect()");
		exit(ERROR);
	}

	debug_sub_msg(debug_mode, "Connected!");

	if (port == 21) {

		char * msg = malloc(5 * sizeof(char));

		debug_sub_msg(debug_mode, "Receiving response message...");

		if (read_from_host(fd, msg, debug_mode, "220") == ERROR) {
			printf("\nNot a valid connect message!\n\n");
			return ERROR;
		}

		debug_sub_msg(debug_mode, "Message received!");
		
		free(msg);

	}

	return fd;
}

int log_in_host(connection * connection, url * url, int debug_mode) {

	char * user = malloc(sizeof(url->user) + 5 * sizeof(char));
	sprintf(user, "user %s\r\n", url->user);

	debug_sub_msg(debug_mode, "Sending user to host...");

	if (send_to_host(connection->fd, user) == ERROR) {
		printf("\t->Error sending a message to host.");
		return ERROR;
	} 

	debug_sub_msg(debug_mode, "User sent!");

	debug_sub_msg(debug_mode, "Receiving message from host...");

	if (read_from_host(connection->fd, user, debug_mode, "331") == ERROR) {
		printf("\nNot a valid user message!\n\n");
		return ERROR;
	}


	debug_sub_msg(debug_mode, "Message received!");


	char * password = malloc(sizeof(url->password) + 5 * sizeof(char));
	sprintf(password, "pass %s\r\n", url->password);

	debug_sub_msg(debug_mode, "Sending password to host...");

	if (send_to_host(connection->fd, password) == ERROR) {
		printf("\t->Error sending a message to host.");
		return ERROR;
	} 

	debug_sub_msg(debug_mode, "Password sent!");

	debug_sub_msg(debug_mode, "Receiving message from host...");

	if (read_from_host(connection->fd, password, debug_mode, "230") == ERROR) {
		printf("\nLog in failed!\n\n");
		return ERROR;
	}

	free(password);

	debug_sub_msg(debug_mode, "Message received!");


	return OK;
}


int pasv_host(connection * connectionA, url * url, int debug_mode, connection * connectionB) {

	char * pasv = malloc(7 * sizeof(char));
	sprintf(pasv, "pasv \r\n");

	debug_sub_msg(debug_mode, "Sending password to host...");

	if (send_to_host(connectionA->fd, pasv) == ERROR) {
		printf("\t->Error sending a message to host.");
		return ERROR;
	} 

	debug_sub_msg(debug_mode, "Password sent!");

	debug_sub_msg(debug_mode, "Interpreting passive message from host...");

	char * ip = malloc(50 * sizeof(char));

	int port;

	if( get_pasv_from_host(connectionA->fd, ip, &port, debug_mode) < 0 ) {
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

	connectionB->ip = ip;
	connectionB->port = port;

	debug_sub_msg(debug_mode, port_info);

	debug_sub_msg(debug_mode, "Completed!");

	return OK;
}

int def_path(connection * connectionA, char * path, int debug_mode) {	
	
	char * retr = malloc(1024 * sizeof(char));
	sprintf(retr, "retr %s\r\n", path);

	char * path_info = malloc(1024 * sizeof(char));
	sprintf(path_info, "File: %s", path);

	debug_sub_msg(debug_mode, path_info);

	debug_sub_msg(debug_mode, "Sending 'retr' command to host...");

	if (send_to_host(connectionA->fd, retr) == ERROR) {
		printf("\t->Error sending a message to host.");
		return ERROR;
	} 

	debug_sub_msg(debug_mode, "Command sent!");

	debug_sub_msg(debug_mode, "Receiving message from host...");

	if (read_from_host(connectionA->fd, retr, debug_mode, "150") == ERROR) {
		printf("\nPath is not valid!\n\n");
		return ERROR;
	}

	free(retr);
	free(path_info);

	debug_sub_msg(debug_mode, "Message received!");
		
	return OK;
}



int disconnect_host(connection * connectionA, url * url, int debug_mode) {
	
	char * quitA = malloc(6 * sizeof(char));
	
	sprintf(quitA, "quit\r\n");

	debug_sub_msg(debug_mode, "Sending 'quit' command to host...");
	
	if (send_to_host(connectionA->fd, quitA) == ERROR) {
		printf("\t->Error sending a message to host A.");
		return ERROR;
	}

	debug_sub_msg(debug_mode, "Closing socket...");

	if (connectionA->fd) {
		close(connectionA->fd);
		free(connectionA);
	}
	
	free(url);

	return OK;
}



int send_to_host(int connection_fd, const char* msg) {
	
	int written_bytes = 0;

	written_bytes = write(connection_fd, msg, strlen(msg));

	int return_value = (written_bytes == strlen(msg)) ? OK : ERROR; 

	return return_value;

}


int read_from_host(int connection_fd, char* msg, int debug_mode, char * code) {

	FILE* fp = fdopen(connection_fd, "r");
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

	if (strcmp(msg, code) != 0) {
		char * error_msg = malloc(1024 * sizeof(char));
		strcpy(error_msg, "\n\nError!! It was suposed to receive a message with the '");
		strcat(error_msg, code);
		strcat(error_msg, "' code, and a it was received a message with the '");
		strcat(error_msg, msg);
		strcat(error_msg, "' code...\n\n");
		
		if(debug_mode)
			printf(error_msg, "\n\nError! You received a wrong code message\n");

		return ERROR;
	}

	return OK;
}



int get_pasv_from_host(int connection_fd, char* ip_str, int * port,int debug_mode) {

	FILE* fp = fdopen(connection_fd, "r");
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


	return OK;
}


int download_from_host(connection * connectionB, char* path, int debug_mode) {
	FILE* file;
	int bytes;

	char * filename = basename(path);

	debug_sub_msg(1, "Creating file with defined path...");

	if (!(file = fopen(filename, "w"))) {
		printf("ERROR: Cannot open file.\n");
		return ERROR;
	}

	debug_sub_msg(1, "Created!");

	debug_sub_msg(1, "Downloading...");

	char buf[1024];
	while ((bytes = read(connectionB->fd, buf, sizeof(buf)))) {
		if (bytes < 0) {
			printf("ERROR: Nothing was received from data socket fd.\n");
			return ERROR;
		}

		if ((bytes = fwrite(buf, bytes, 1, file)) < 0) {
			printf("ERROR: Cannot write data in file.\n");
			return ERROR;
		}
	}

	debug_sub_msg(1, "Completed!");

	if(connectionB->fd) {
		close(connectionB->fd);
		free(connectionB);
	}

	if(file)
	 	fclose(file);

	return 0;
}


