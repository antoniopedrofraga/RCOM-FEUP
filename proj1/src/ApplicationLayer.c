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
#include "Cli.h"

ApplicationLayer* al;

int initAppLayer(char* port, int status, char * filePath,int timeout, int retries, int pktSize, int baudrate) {
	al = (ApplicationLayer*) malloc(sizeof(ApplicationLayer));

	al->fd = openSerialPort(port);
	
	if (al->fd < 0) {
		printf("ERROR in initAppLayer(): could not open serial port\n");
		return ERROR;
	}
	al->status = status;

	al->file = openFile(filePath);
	
	int fileSize;

	if (al->status == TRANSMITTER) {
		struct stat st; 
		if (stat(filePath, &st) == 0)
			fileSize = st.st_size;
		else {
			printf("ERROR getting file size!\n");
			return ERROR;
		}
	}
	
	if (al->file == NULL )
		return ERROR;	

	if (initLinkLayer(port, baudrate, pktSize, timeout, retries) < 0) {
		printf("ERROR in initAppLayer(): could not initialize link layer\n");
		return ERROR;
	}
	
	printWaiting(al->status);
	
	if (llopen() == ERROR)
		return ERROR;

	if (al->status == TRANSMITTER)
		sendData(filePath, fileSize);
	else if (al->status == RECEIVER)
		receiveData(filePath);
	
	llclose();
	closeSerialPort();
	
	printStatistics();

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

	return file;	
}

int sendData(char * filePath, int fileSize) {

	if (sendCtrlPkt(CTRL_PKT_START, filePath, fileSize) < 0)
		return ERROR;
	
	ll->statistics.msgSent++;

	int bytesRead = 0, i = 0, bytesAcumulator = 0;;
	char * buffer = malloc(ll->pktSize * sizeof(char));

	while((bytesRead = fread(buffer, sizeof(char), ll->pktSize, al->file)) > 0){
		if(sendDataPkt(buffer, bytesRead, i) < 0)
			return ERROR;

		ll->statistics.msgSent++;
		i++;
		if (i > 207)
			i = 0;
		bytesAcumulator += bytesRead;
		printProgressBar(filePath, bytesAcumulator, fileSize, 0);
	}

	if (fclose(al->file) < 0) {
		printf("ERROR in sendData(): error closing file!\n");
		return ERROR;
	}

	if (sendCtrlPkt(CTRL_PKT_END, filePath, fileSize) < 0)
		return ERROR;

	ll->statistics.msgSent++;

	printf("File sent!\n");

	return 0;
}



int receiveData(char * filePath) {
	int fileSize;

	if(rcvCtrlPkt(CTRL_PKT_START, &fileSize, &filePath) < 0)
		return ERROR;

	ll->statistics.msgRcvd++;

	int bytesRead, bytesAcumulator = 0, i = 0;
	unsigned char * buffer = malloc(ll->pktSize * sizeof(char));

	while (bytesAcumulator < fileSize){
		bytesRead = rcvDataPkt(&buffer, i);
		printf("%d\n", bytesRead);
		if(bytesRead < 0)
			return ERROR;
		ll->statistics.msgRcvd++;
		bytesAcumulator += bytesRead;
		fwrite(buffer, sizeof(char), bytesRead, al->file);
		i++;
		if (i > 207)
			i = 0;
		printProgressBar(filePath, bytesAcumulator, fileSize, 1);
	}

	if (fclose(al->file) < 0) {
		printf("ERROR in receiveData(): error closing file!\n");
		return ERROR;
	}

	if (rcvCtrlPkt(CTRL_PKT_END, &fileSize, &filePath) < 0)
		return ERROR;

	ll->statistics.msgRcvd++;

	printf("File received!\n");

	return 0;
}


int sendCtrlPkt(int ctrlField, char * filePath, int fileSize) {

	char sizeString[16];
	sprintf(sizeString, "%d", fileSize);

	int size = 5 + strlen(sizeString) + strlen(filePath);

	unsigned char ctrlPckg[size];

	ctrlPckg[0] = ctrlField + '0';
	ctrlPckg[1] = PARAM_SIZE + '0';
	ctrlPckg[2] = strlen(sizeString) + '0';


	int i, acumulator = 3;
	for(i = 0; i < strlen(sizeString); i++) {
		ctrlPckg[acumulator] = sizeString[i];
		acumulator++;;
	}

	ctrlPckg[acumulator] = PARAM_NAME + '0';
	acumulator++;
	ctrlPckg[acumulator] = strlen(filePath) + '0';
	acumulator++;

	for(i = 0; i < strlen(filePath); i++) {
		ctrlPckg[acumulator] = filePath[i];
		acumulator++;;
	}

	if (llwrite(ctrlPckg, size) < 0) {
		printf("ERROR in sendCtrlPkt(): llwrite() function error!\n");
		return ERROR;
	}

	return 0;
}


int rcvCtrlPkt(int controlField, int * fileSize, char ** filePath) {

	unsigned char * info;

	if (llread(&info) < 0) {
		printf("ERROR in rcvCtrlPkt(): \n");
		return ERROR;
	}
	
	if ((info[0] - '0') != controlField) {
		printf("ERROR in rcvCtrlPkt(): unexpected control field!\n");
		return ERROR;
	}

	if ((info[1] - '0') != PARAM_SIZE) {
		printf("ERROR in rcvCtrlPkt(): unexpected size param!\n");
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
		printf("ERROR in rcvCtrlPkt(): unexpected name param!\n");
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


int sendDataPkt(char * buffer, int bytesRead, int i) {

	int size = bytesRead + 4;
	unsigned char dataPckg[size];

	dataPckg[0] = CTRL_PKT_DATA + '0';
	dataPckg[1] = i + '0';

	dataPckg[2] = bytesRead / 256;
	dataPckg[3] = bytesRead % 256;
	memcpy(&dataPckg[4], buffer, bytesRead);

	if (llwrite(dataPckg, size) < 0) {
		printf("ERROR in sendDataPkt(): llwrite() function error!\n");
		return ERROR;
	}

	return 0;
}

int rcvDataPkt(unsigned char ** buffer,int i) {

	unsigned char * info = NULL;
	int bytes = 0;

	if (llread(&info) < 0) {
		printf("ERROR in rcvDataPkt(): llread() function error!\n");
		return ERROR;
	}

	if (info == NULL)
		return 0;

	int C = info[0] - '0';
	int N = info[1] - '0';

	if (C != CTRL_PKT_DATA) {
		printf("ERROR in rcvDataPkt(): control field it's different from CTRL_PKT_DATA!\n");
		return ERROR;
	}
	
	if (N != i) {
		printf("ERROR in rcvDataPkt(): sequence number it's wrong!\n");
		return ERROR;
	}

	int L2 = info[2], L1 = info[3];
	bytes = 256 * L2 + L1;

	memcpy((*buffer), &info[4], bytes);

	free(info);

	return bytes;
}

void printStatistics() {
	printf("\n");
	printf("### Statistics ###\n\n");
	printf("Timeouts: %d\n\n", ll->statistics.timeout);
	printf("Sent messages: %d\n", ll->statistics.msgSent);
	printf("Received messages: %d\n\n", ll->statistics.msgRcvd);
	printf("Sent RR: %d\n", ll->statistics.rrSent);
	printf("Received RR: %d\n\n", ll->statistics.rrRcvd);
	printf("Sent REJ: %d\n", ll->statistics.rejSent);
	printf("Received REJ: %d\n\n", ll->statistics.rejRcvd);
}


