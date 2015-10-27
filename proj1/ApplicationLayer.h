#pragma once

typedef struct {
	int fd;
	int status;
	FILE * file;
	int fileSize;
} ApplicationLayer;

extern ApplicationLayer* al;

typedef enum {
	PARAM_SIZE = 0, PARAM_NAME = 1
} CtrlPckgParam;

int initAppLayer(char * port, int status, char * filePath, int timeout, int retries, int pkgSize, int baudrate);

FILE * openFile(char * filePath);

int sendData(char * filePath);

int receiveData(char * filePath);

int sendCtrlPkg(int ctrlField, char * filePath);

int rcvCtrlPkg(int controlField, int * fileSize, char ** filePath);

int sendDataPkg(char * buffer, int bytesRead, int i);

int rcvDataPkg(unsigned char ** buffer,int i);

void printStatistics();


