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
#include "ApplicationLayer.h"
#include "Alarm.h"

LinkLayer* ll;

int initLinkLayer(char* port, int baudRate, unsigned int timeout, unsigned int numRetries) {
	ll = (LinkLayer*) malloc(sizeof(LinkLayer));

	strcpy(ll->port, port);
	ll->baudRate = baudRate;
	ll->sequenceNumber = 0;
	ll->timeout = timeout;
	ll->numRetries = numRetries;

	if (setNewTermios() < 0)
    	return ERROR;

    return 0;
}

int openSerialPort(char* port) {
	return open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
}

int closeSerialPort() {
	// set old settings
	if (tcsetattr(al->fd, TCSANOW, &ll->oldtio) < 0) {
		printf("ERROR in closeSerialPort(): could not set old termios\n");
		return ERROR;
	}

	close(al->fd);

	return 0;
}

int setNewTermios() {
	// save current port settings
	if (tcgetattr(al->fd, &ll->oldtio) < 0) {
		printf("ERROR in setNewTermios(): could not get old termios\n");
		return ERROR;
	}
    
    // set new termios
    bzero(&ll->newtio, sizeof(ll->newtio));
    ll->newtio.c_cflag = ll->baudRate | CS8 | CLOCAL | CREAD;
    ll->newtio.c_iflag = IGNPAR;
    ll->newtio.c_oflag = 0;

 
    ll->newtio.c_lflag = 0;

    ll->newtio.c_cc[VTIME] = 0;   
    ll->newtio.c_cc[VMIN] = 1;   


    if (tcflush(al->fd, TCIOFLUSH) < 0) {
    	printf("ERROR in setNewTermios(): could not flush non-transmitted output data\n");
    	return ERROR;
    }

    if (tcsetattr(al->fd, TCSANOW, &ll->newtio) < -1) {
    	printf("ERROR in setNewTermios(): could not set new termios\n");
    	return ERROR;
    }
}

int llopen(int mode) {
	int counter = 0;

	switch(mode){
		case TRANSMITTER:
			setAlarm();

			while(counter < ll->numRetries) {
				if (counter == 0 || alarmFired) {
					alarmFired = 0;
					printf("Sending message...\n");
					sendCommand(al->fd, SET);
					counter++;
					if (receiveCommand(al->fd) == 0)
						break;
				}
			}

			stopAlarm();
			if (counter < ll->numRetries)
				printf("Connection successfully established!\n");
			else {
				printf("ERROR in llopen(): could not establish a connection\n");
				return ERROR;
			}
			break;
		case RECEIVER:
			if (receiveCommand(al->fd) == 0) {
				sendCommand(al->fd, UA);
				printf("Connection successfully established!\n");
			}
			break;
		default:
			break;
	}

	return 0;
}

int sendCommand(int fd, Command cmd) {
	unsigned char command[COMMAND_SIZE];

	command[0] = FLAG;
	command[1] = A;
	command[4] = FLAG;

	switch(cmd) {
		case SET:
			command[2] = C_SET;
			command[3] = command[1] ^ command[2];
			break;
		case UA:
			command[2] = C_UA;
			command[3] = command[1] ^ command[2];
			break;
		default:
			printf("ERROR in sendCommand(): unexpected command\n");
			break;
	}

	if (write(fd, command, COMMAND_SIZE) != COMMAND_SIZE) {
		printf("ERROR in sendCommand(): could not send\n");
		return ERROR;
	}

	return 0;
}

int receiveCommand(int fd) {
	unsigned char c;
	char tmp[5];
	int res;
	int state = 0;

	tcflush(fd, TCIFLUSH);
	
	while(state < 5)
	{
		res = read(fd, &c, 1);
		if (res > 0)
			state = stateMachine(c, state, tmp);
	}

	return 0;
}

int stateMachine(unsigned char c, int state, char cmd[])
{
	switch(state) {
		case 0:
			if (c == FLAG) {
				cmd[state] = c;
				state++;
			}
			break;
		case 1: 
			if (c == A) {
				cmd[state] = c;
				state++;
			}
			else if (c == FLAG)
				state = 1;
			else
				state = 0;
			break;
		case 2: 
			if (c != FLAG) {
				cmd[state] = c;
				state++;
			}
			else if (c == FLAG)
				state = 1;
			else
				state = 0;
			break;
		case 3:
			if (c == (cmd[1]^cmd[2])) {
				cmd[state] = c;
				state++;
			}
			else if (c == FLAG)
				state = 1;
			else
				state = 0;
			break;
		case 4:
			if (c == FLAG) {
				cmd[state] = c;
				state++;
			}
			else
				state = 0;
			break;
		default:
			break;
	}

	return state;	
}