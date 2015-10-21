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

int initAppLayer(char* port, int status) {
	al = (ApplicationLayer*) malloc(sizeof(ApplicationLayer));

	al->fd = openSerialPort(port);
	if(al->fd < 0) {
		printf("ERROR IN initAppLayer(): could not open serial port\n");
		return ERROR;
	}
	al->status = status;

	if (initLinkLayer(port, BAUDRATE, 3, 3) < 0) {
		printf("ERROR in initAppLayer(): could not initialize link layer\n");
		return ERROR;
	}

	closeSerialPort();

	return 0;
}