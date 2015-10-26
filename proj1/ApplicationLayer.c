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
	
	if (al->fd < 0) {
		printf("ERROR in initAppLayer(): could not open serial port\n");
		return ERROR;
	}
	al->status = status;

	al->file = openFile(filePath);
	
	if (al->file == NULL )
		return ERROR;	

	if (initLinkLayer(port, BAUDRATE, 3, 3) < 0) {
		printf("ERROR in initAppLayer(): could not initialize link layer\n");
		return ERROR;
	}
	
	if (llopen() == ERROR)
		return ERROR;

	if (al->status == TRANSMITTER)
		sendData(filePath);
	else if (al->status == RECEIVER)
		receiveData(filePath);

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

int sendData(char * filePath) {

	if (sendCtrlPkg(CTRL_PKG_START, filePath) < 0)
		return ERROR;

	printf("Sent control package.\n");

	llclose();
	return 0;
}



int receiveData(char * filePath) {
	int fileSize;
	if(rcvCtrlPkg(CTRL_PKG_START, &fileSize, &filePath) < 0)
		return ERROR;

	printf("Received control package.\nFile size = %d bytes/ File Name  = %s\n", fileSize, filePath);

	llclose();

	return 0;
}


int sendCtrlPkg(int ctrlField, char * filePath) {

	char sizeString[16];
	sprintf(sizeString, "%d", al->fileSize);

	int size = 5 + strlen(sizeString) + strlen(filePath);

	unsigned char ctrlPckg[size];

	ctrlPckg[0] = ctrlField + '0';
	ctrlPckg[1] = PARAM_SIZE + '0';
	ctrlPckg[2] = strlen(sizeString) + '0';

	//printf("paramSize = %c, %c, %c\n", ctrlPckg[0], ctrlPckg[1], ctrlPckg[2]);

	int i, acumulator = 3;
	for(i = 0; i < strlen(sizeString); i++) {
		ctrlPckg[acumulator] = sizeString[i];
		//printf("%c\n", ctrlPckg[acumulator]);
		acumulator++;;
	}

	ctrlPckg[acumulator] = PARAM_NAME + '0';
	acumulator++;
	ctrlPckg[acumulator] = strlen(filePath) + '0';
	acumulator++;

	//printf("Param name = %c, %c\n\nName string:\n", ctrlPckg[acumulator - 2], ctrlPckg[acumulator - 1]);

	//printf("File Path Size= %d\n", (int) strlen(filePath));

	for(i = 0; i < strlen(filePath); i++) {
		ctrlPckg[acumulator] = filePath[i];
		//printf("%c\n", ctrlPckg[acumulator]);
		acumulator++;;
	}

	if (llwrite(ctrlPckg, size) < 0) {
		printf("ERROR in sendCtrlPckg(): \n");
		return ERROR;
	}

	return 0;
}


int rcvCtrlPkg(int controlField, int * fileSize, char ** filePath) {

	unsigned char * info;

	if (llread(&info) < 0) {
		printf("ERROR in rcvCtrlPckg(): \n");
		return ERROR;
	}
	
	if ((info[0] - '0') != controlField) {
		printf("ERROR in rcvCtrlPckg(): unexpected control field!\n");
		return ERROR;
	}

	if ((info[1] - '0') != PARAM_SIZE) {
		printf("ERROR in rcvCtrlPckg(): unexpected size param!\n");
		return ERROR;
	}

	int i, fileSizeLength = (info[2] - '0'), acumulator = 3;

	char fileSizeStr[MAX_STR_SIZE];

	for(i = 0; i < fileSizeLength; i++) {
		fileSizeStr[i] = info[acumulator];
		acumulator++;
	}

	fileSizeStr[acumulator - 3] = '\0';

	(*fileSize) = atoi(fileSizeStr);

	if((info[acumulator] - '0') != PARAM_NAME) {
		printf("ERROR in rcvCtrlPckg(): unexpected name param!\n");
		return ERROR;
	}

	acumulator++;
	
	int pathLength = (info[acumulator] - '0');
	acumulator++;

	char pathStr[MAX_STR_SIZE];
	
	for(i = 0; i < pathLength; i++) {
		pathStr[i] = info[acumulator];
		acumulator++;
	}

	pathStr[i] = '\0';
	strcpy((*filePath), pathStr);

	return 0;
}
