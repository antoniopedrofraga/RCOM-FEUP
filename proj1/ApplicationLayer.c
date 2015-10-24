#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>


#include "DataLinkLayer.h"
#include "Utilities.h"
#include "ApplicationLayer.h"

ApplicationLayer* al;

int initAppLayer(char* port, int status, char * filePath) {
	al = (ApplicationLayer*) malloc(sizeof(ApplicationLayer));

	al->fd = openSerialPort(port);
	
	if(al->fd < 0) {
		printf("ERROR in initAppLayer(): could not open serial port\n");
		return ERROR;
	}
	al->status = status;

	al->file = openFile(filePath);
	
	if(al->file == NULL ) return ERROR;	

	if (initLinkLayer(port, BAUDRATE, 3, 3) < 0) {
		printf("ERROR in initAppLayer(): could not initialize link layer\n");
		return ERROR;
	}
	
	if(llopen() == ERROR) return ERROR;

	if(al->status == TRANSMITTER) sendData();
	else if(al->status == RECEIVER) ;

	llclose();

	closeSerialPort();

	return 0;
}

FILE * openFile(char * filePath) {
	
	FILE * file;
	
	if(al->status == TRANSMITTER) file = fopen(filePath, "rb");
	else file = fopen(filePath, "wb");
		
	if(file == NULL) {
		printf("ERROR in openFile(): error opening file with path <%s>\n", filePath);
		return NULL;
	}
	
	struct stat st; 

    	if (stat(filePath, &st) == 0)
        	al->fileSize = st.st_size;
	else {
		printf("ERROR in openFile(): error getting file size\n");
		return NULL;
	}

	return file;	
}

int sendData() {
	return 0;
}



