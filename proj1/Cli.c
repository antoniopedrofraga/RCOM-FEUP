#include <stdio.h>
#include <stdlib.h>
#include "Cli.h"
#include "Utilities.h"


int getMode() {
	int mode = ERROR;
	while (mode != 1 && mode != 2) {
		clrscr();
		printf("# Do you want to receive or send data?\n\n");
		printf("\t1. Send\n");
		printf("\t2. Receive\n\n> ");
		mode = getchar() - '0';
	}
	return mode - 1;		
}


char * getPort() {
	int port = ERROR;
	while (port != 1 && port != 2) {
		clrscr();
		printf("# Which port do you want to use?\n\n");
		printf("\t1. /dev/ttyS0\n");
		printf("\t2. /dev/ttyS4\n\n> ");
		port = getchar() - '0';
	}
	
	if (port == 1)
		return "/dev/ttyS0";
	else
		return "/dev/ttyS4";
}


char * getFileName(int mode) {
	char * fileName = malloc(150*sizeof(char));;
	clrscr();
	if (mode == 0)
		printf("# Type the name of file to be read: \n\n> ");
	else
		printf("# Type the name of the output file: \n\n> ");

	scanf("%s", fileName);

	return fileName;
}

int getRetries() {
	int retries = ERROR;
	while (retries <= 0) {
		clrscr();
		printf("# Which is the maximum number of retries to send a package?\n\n> ");
	
		scanf("%d", &retries);
	}

	return retries;
}

int getTimeout() {
	int timeout = ERROR;
	while (timeout <= 0) {
		clrscr();
		printf("# Which is the timeout waiting time in seconds?\n\n> ");
	
		scanf("%d", &timeout);
	}

	return timeout;
}

void printProgressBar(char * fileName, int bytes, int size, int mode) {
	clrscr();
	if (mode == 0)
		printf("Sending %s...\n\n", fileName);
	else if (mode == 1)
		printf("Receiving %s...\n\n", fileName);
	
	printf("[");
	int i, barSize = 30;
	for (i = 0; i < barSize; i++) {
		if(((float)bytes / (float)size ) > ( (float)i / barSize))
			printf("=");
		else
			printf(" ");
	}
	printf("]");
	printf("  %d %%\t%d / %d bytes\n\n", (int)((float)bytes / (float)size * 100), bytes, size);
}

void printWaiting(int mode) {
	clrscr();
	if (mode == 0)
		printf("Waiting for receiver...\n\n");
	else
		printf("Waiting for transmitter...\n\n");	
}

void clrscr() {
	printf("\033[2J");
} 
