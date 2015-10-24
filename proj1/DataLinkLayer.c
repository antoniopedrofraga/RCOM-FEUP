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

int llopen() {
	int counter = 0;

	switch(al->status){
		case TRANSMITTER:
			while(counter < ll->numRetries) {
				if (counter == 0 || alarmFired) {
					alarmFired = 0;
					sendFrame(al->fd, SET);
					counter++;

					setAlarm();
				}

				if (receiveFrame(al->fd) == 0) {
					counter--;
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


int llclose() {
	int counter = 0;

	switch(al->status){
		case TRANSMITTER:
			while(counter < ll->numRetries) {
				if (counter == 0 || alarmFired) {
					alarmFired = 0;
					sendFrame(al->fd, DISC);
					counter++;
					setAlarm();
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
				while(counter < ll->numRetries) {
					if (counter == 0 || alarmFired) {
						alarmFired = 0;
						counter++;
						setAlarm();
						sendFrame(al->fd, DISC);
						if(receiveFrame(al->fd) == 0) {
							printf("Connection successfully disconected!\n");
							break;
						}else
							printf("erro a receber frame\n");
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

unsigned char getBCC2(unsigned char* data, unsigned int size) {
	unsigned char BCC;

	int i;
	for (i = 0; i < size; i++)
		BCC ^= data[i];

	return BCC;
}

int sendDataFrame(int fd, unsigned char* data, unsigned int size) {
	DataFrame df;

	df.size =  size + DATA_FRAME_SIZE;

	df.frame[0] = FLAG;
	df.frame[1] = A03;
	df.frame[2] = ll->sequenceNumber << 5;
	df.frame[3] = df.frame[1] ^ df.frame[2];
	memcpy(&df.frame[4], data, size);
	df.frame[4 + size] = getBCC2(data, size);
	df.frame[5 + size] = FLAG;

	df = stuff(df);

	if (write(fd, df.frame, df.size) != df.size) {
		printf("ERROR in sendDataFrame(): could not send frame\n");
		return ERROR;
	}

	return 0;
}

int sendFrame(int fd, Command cmd) {
	unsigned char frame[FRAME_SIZE];

	frame[0] = FLAG;
	frame[4] = FLAG;

	switch(cmd) {
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
		printf("ERROR in sendFrame(): could not send frame\n");
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
	int res, receiving = 1, state = 0, dataFrame = 0, i = 0;
	DataFrame df;

	while(receiving) {
		res = read(fd, &c, 1);
		
		if (res < 1)
			return ERROR;

		switch(state) {
		case 0:
			if (c == FLAG) {
				df.frame[i] = c;
				i++;
				state++;
			}
			break;
		case 1:
			if (c == A01 || c == A03) {
				df.frame[i] = c;
				i++;
				state++;
			}
			else if (c == FLAG)
				state = 1;
			else
				state = 0;
			break;
		case 2: 
			if (c != FLAG) {
				df.frame[i] = c;
				i++;
				state++;
			}
			else if (c == FLAG)
				state = 1;
			else
				state = 0;
			break;
		case 3:
			if (c == (ll->frame[1]^ll->frame[2])) {
				df.frame[i] = c;
				i++;
				state++;
			}
			else if (c == FLAG)
				state = 1;
			else
				state = 0;
			break;
		case 4:
			if (c == FLAG) {
				df.frame[i] = c;
				state++;

				if (i > 4)
					dataFrame = 1;
			}
			else {
				df.frame[i] = c;
				i++;
			}
			break;
		case 5:
			receiving = 0;
			break;
		default:
			break;
		}
	}

	if (dataFrame) {
		df.size = i;
		df = destuff(df);
		
		// check BCC1
		if (df.frame[3] != (df.frame[1] ^ df.frame[2]))
			df.status = REJ;
		
		// check BCC2
		int dataSize = df.size - DATA_FRAME_SIZE;
		unsigned char BCC2 = getBCC2(&(df.frame[4]), dataSize);
		if (df.frame[4 + dataSize] != BCC2)
			df.status = REJ;
		
			
	}

	return 0;
}

DataFrame stuff(DataFrame df) {
	DataFrame stuffedFrame;
	unsigned int newSize = df.size;

	int i;
	for (i = 1; i < df.size - 1; i++) {
		if (df.frame[i] == FLAG || df.frame[i] == ESCAPE)
			newSize++;
	}

	stuffedFrame.size = newSize;
	stuffedFrame.frame[0] = df.frame[0];

	int j = 1;
	for (i = 1; i < df.size - 1; i++) {
		if (df.frame[i] == FLAG || df.frame[i] == ESCAPE) {
			stuffedFrame.frame[j] = ESCAPE;
			stuffedFrame.frame[++j] = df.frame[i] ^ 0x20;
		}
		else
			stuffedFrame.frame[j] = df.frame[i];
		j++;
	}

	stuffedFrame.frame[j] = df.frame[i];

	return stuffedFrame;

}

DataFrame destuff(DataFrame df) {
	DataFrame destuffedFrame;
	int j = 0;

	int i;
	for (i = 0; i < df.size; i++) {
		if (df.frame[i] == ESCAPE)
			destuffedFrame.frame[j] = df.frame[++i] ^ 0x20;
		else
			destuffedFrame.frame[j] = df.frame[i];
		j++;
	}

	destuffedFrame.size = j;

	return destuffedFrame;
}
