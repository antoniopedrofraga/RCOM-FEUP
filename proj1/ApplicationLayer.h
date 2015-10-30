#pragma once

typedef struct {
	int fd;
	int status;
	FILE * file;
} ApplicationLayer;

extern ApplicationLayer* al;

typedef enum {
	PARAM_SIZE = 0, PARAM_NAME = 1
} CtrlPckgParam;

int initAppLayer(char * port, int status, char * filePath, int timeout, int retries, int pktSize, int baudrate);

FILE * openFile(char * filePath);

int sendData(char * filePath, int fileSize);
int receiveData(char * filePath);

int sendCtrlPkt(int ctrlField, char * filePath, int fileSize);
int rcvCtrlPkt(int controlField, int * fileSize, char ** filePath);

int sendDataPkt(char * buffer, int bytesRead, int i);
int rcvDataPkt(unsigned char ** buffer,int i);

void printStatistics();


