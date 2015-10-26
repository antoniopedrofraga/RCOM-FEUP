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

	if (al->status == TRANSMITTER) {
		if(sendData(filePath) < 0) {
			printf("ERROR sending data\n");
			return ERROR;
		}
	} else if (al->status == RECEIVER) {
		if(receiveData(filePath) < 0) {
			printf("ERROR receiving data\n");
			return ERROR;
		}
	}

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
		printf("ERROR in openFile(): error getting file size!\n");
		return NULL;
	}

	return file;	
}

int sendData(char * filePath) {

	if (sendCtrlPkg(CTRL_PKG_START, filePath) < 0)
		return ERROR;

	printf("Control Package sent!\n");

	/*int bytesRead = 0, i = 0;
	char * buffer = malloc(MAX_BUF_SIZE * sizeof(char));

	bytesRead = fread(buffer, sizeof(char), MAX_BUF_SIZE, al->file);
	printf("bytesRead begining = %d\n", bytesRead);
	sendDataPkg(buffer, bytesRead, i);*/

	/*while((bytesRead = fread(buffer, sizeof(char), MAX_BUF_SIZE, al->file)) > 0){
		printf("bytesRead begining = %d\n", bytesRead);
		if(sendDataPkg(buffer, bytesRead, i) < 0) {
			printf("ERROR in sendData(): error sending data package!\n");
			return ERROR;
		}
		printf("after sendDataPkg\n");
		i++;
	}
	printf("bytesRead end = %d\n", bytesRead);

	if (fclose(al->file) < 0) {
		printf("ERROR in sendData(): error closing file!\n");
		return ERROR;
	}

	if (sendCtrlPkg(CTRL_PKG_END, filePath) < 0)
		return ERROR;*/

	return 0;
}



int receiveData(char * filePath) {
	int fileSize;
	if(rcvCtrlPkg(CTRL_PKG_START, &fileSize, &filePath) < 0)
		return ERROR;

	printf("Control Package received!\n");
	printf("File size = %d, File name = %s\n", fileSize, filePath);

	/*int bytesRead, bytesAcumulator = 0, i = 0;
	unsigned char * buffer = malloc(MAX_BUF_SIZE * sizeof(char));;
	while(bytesAcumulator < fileSize){
		bytesRead = rcvDataPkg(&buffer, i);
		if(bytesRead < 0) {
			return ERROR;
		}
		bytesAcumulator += bytesRead;
		fwrite(buffer, sizeof(char), bytesRead, al->file);
		i++;
	}

	if (fclose(al->file) < 0) {
		printf("ERROR in senData(): error closing file!\n");
		return ERROR;
	}

	if(rcvCtrlPkg(CTRL_PKG_END, &fileSize, &filePath) < 0)
		return ERROR;*/

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
		printf("ERROR in sendCtrlPkg(): llwrite() function error!\n");
		return ERROR;
	}

	return 0;
}


int rcvCtrlPkg(int controlField, int * fileSize, char ** filePath) {

	unsigned char * info;

	if (llread(&info) < 0) {
		printf("ERROR in rcvCtrlPkg(): \n");
		return ERROR;
	}
	
	if ((info[0] - '0') != controlField) {
		printf("ERROR in rcvCtrlPkg(): unexpected control field!\n");
		return ERROR;
	}

	if ((info[1] - '0') != PARAM_SIZE) {
		printf("ERROR in rcvCtrlPkg(): unexpected size param!\n");
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
		printf("ERROR in rcvCtrlPkg(): unexpected name param!\n");
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


int sendDataPkg(char * buffer, int bytesRead, int i) {

	int size = bytesRead + 4;
	unsigned char dataPckg[size];

	dataPckg[0] = CTRL_PKG_DATA + '0';
	dataPckg[1] = i + '0';

	int L1 = bytesRead / MAX_BUF_SIZE;
	int L2 = bytesRead % MAX_BUF_SIZE;

	dataPckg[2] = L1 + '0';
	dataPckg[3] = L2 + '0';
	memcpy(&dataPckg[4], buffer, bytesRead);
	
	printf("0 = %c, 1 = %c, 2 = %c, 3 = %c, size = %d\n", dataPckg[0], dataPckg[1], dataPckg[2], dataPckg[3], bytesRead);

	if (llwrite(dataPckg, size) < 0) {
		printf("ERROR in sendDataPkg(): llwrite() function error!\n");
		return ERROR;
	}
	return 0;
}

int rcvDataPkg(unsigned char ** buffer,int i) {

	unsigned char * info;
	int bytes = 0;

	if (llread(&info) < 0) {
		printf("ERROR in rcvDataPkg(): llread() function error!\n");
		return ERROR;
	}


	int C = info[0] - '0';
	int N = info[1] - '0';
	int L1 = info[2] - '0';
	int L2 = info[3] - '0';

	printf("C = %d, C char = %c, ctrl_pckg_data = %d\n", C, info[0],(int)CTRL_PKG_DATA);

	printf("L1 = %d, L2 = %d\n", L1, L2);

	if(C != CTRL_PKG_DATA) {
		printf("ERROR in rcvDataPkg(): control field it's different from CTRL_PKG_DATA!\n");
		return ERROR;
	}
	if(N != i) {
		printf("ERROR in rcvDataPkg(): sequence number it's wrong!\n");
		return ERROR;
	}

	bytes = 256 * L2 + L1;

	memcpy((*buffer), &info[4], bytes);

	free(info);

	return bytes;
}
