#pragma once

typedef struct {
	int fd;
	int status;
	FILE * file;
	int fileSize;
} ApplicationLayer;

extern ApplicationLayer* al;

int initAppLayer(char* port, int status, char * filePath);

FILE * openFile(char * filePath);

int sendData();
