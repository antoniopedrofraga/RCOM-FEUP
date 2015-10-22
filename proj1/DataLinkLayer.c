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
	return open(port, O_RDWR | O_NOCTTY);
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
    return 0;
}

int llopen(int mode) {
	int counter = 0;

	switch(mode){
		case TRANSMITTER:
			while(counter < ll->numRetries) {
				if (counter == 0 || alarmFired) {
					alarmFired = 0;
					printf("Sending message...\n");
					sendFrame(al->fd, SET);
					counter++;

					if(counter != 0)	
						setAlarm();
				}

				if (receiveFrame(al->fd) == 0)
					break;

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
			if (receiveFrame(al->fd) == 0) {
				sendFrame(al->fd, UA);
				printf("Connection successfully established!\n");
			}
			break;
		default:
			break;
	}

	return 0;
}


int llclose(int mode){
	int counter = 0;

	switch(mode){
		case TRANSMITTER:
			setAlarm();

			while(counter < ll->numRetries) {
				if (counter == 0 || alarmFired) {
					alarmFired = 0;
					sendFrame(al->fd, DISC);
					counter++;
					if (receiveFrame(al->fd) == 0){
						sendFrame(al->fd, UA);
						break;
					}
				}
			}

			stopAlarm();
			if (counter < ll->numRetries)
				printf("Connection successfully disconected!\n");
			else {
				printf("ERROR in llclose(): could not disconect\n");
				return ERROR;
			}
			break;
		case RECEIVER:
			if (receiveFrame(al->fd) == 0) {

				setAlarm();

				while(counter < ll->numRetries) {
					if (counter == 0 || alarmFired) {
						alarmFired = 0;
						counter++;
						sendFrame(al->fd, DISC);
						if(receiveFrame(al->fd) == 0) {
							printf("Connection successfully disconected!\n");
							break;
						}
					}
				}

				stopAlarm();

			}
			break;
		default:
			break;
	}

	return 0;
}

int sendFrame(int fd, Frame frm) {
	unsigned char frame[FRAME_SIZE];

	frame[0] = FLAG;
	frame[4] = FLAG;

	switch(frm) {
		case SET:
			frame[1] = getAFromCmd();
			frame[2] = C_SET;
			frame[3] = frame[1] ^ frame[2];
			break;
		case UA:
			frame[1] = getAFromRspn();
			frame[2] = C_UA;
			frame[3] = frame[1] ^ frame[2];
			break;
		case DISC:
			frame[1] = getAFromCmd();
			frame[2] = C_DISC;
			frame[3] = frame[1] ^ frame[2];
			break;
		default:
			printf("ERROR in sendFrame(): unexpected frame\n");
			break;
	}

	if(frame[1] == 0) {
		printf("ERROR in sendFrame(): unnexpected status\n");
		return ERROR;
	}

	if (write(fd, frame, FRAME_SIZE) != FRAME_SIZE) {
		printf("ERROR in sendFrame(): could not send\n");
		return ERROR;
	}

	return 0;
}

unsigned char getAFromCmd() {
	switch(al->status){
		case TRANSMITTER:
			return A03;
		case RECEIVER:
			return A01;
	}
	return 0;
}

unsigned char getAFromRspn() {
	switch(al->status){
		case TRANSMITTER:
			return A01;
		case RECEIVER:
			return A03;
	}
	return 0;
}

int receiveFrame(int fd) {
	unsigned char c;
	char tmp[5];
	int res;
	int state = 0;

	while(state < 5)
	{
		res = read(fd, &c, 1);

		if (res > 0)
			state = stateMachine(c, state, tmp);
		else
			return ERROR;

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
			if (c == A01 || c == A03) {
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