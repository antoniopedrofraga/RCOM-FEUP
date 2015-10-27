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

int initLinkLayer(char* port, int baudRate,  unsigned int pkgSize,unsigned int timeout, unsigned int numRetries) {
	ll = (LinkLayer*) malloc(sizeof(LinkLayer));

	strcpy(ll->port, port);
	ll->baudRate = baudRate;
	ll->sn = 0;
	ll->timeout = timeout;
	ll->numRetries = numRetries;
	ll->pkgSize = pkgSize;

	ll->statistics.timeout = 0;
	ll->statistics.msgSent = 0;
	ll->statistics.msgRcvd = 0;
	ll->statistics.rrSent = 0;
	ll->statistics.rrRcvd = 0;
	ll->statistics.rejSent = 0;
	ll->statistics.rejRcvd = 0;	

	if (setNewTermios() < 0)
    	return ERROR;

    return 0;
}

int getBaudrateChoice(int choice) {
	switch (choice) {
	case 0:
		return B0;
	case 50:
		return B50;
	case 75:
		return B75;
	case 110:
		return B110;
	case 134:
		return B134;
	case 150:
		return B150;
	case 200:
		return B200;
	case 300:
		return B300;
	case 600:
		return B600;
	case 1200:
		return B1200;
	case 1800:
		return B1800;
	case 2400:
		return B2400;
	case 4800:
		return B4800;
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	case 230400:
		return B230400;
	case 460800:
		return B460800;
	default:
		return -1;
	}
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
					sendCommand(al->fd, SET);
					counter++;

					setAlarm();
				}

				if (isCommand(receiveFrame(al->fd), UA)) {
					counter--;
					break;
				}
			}

			stopAlarm();
			if (counter < ll->numRetries)
				printf("Connection successfully established!\n");
			else {
				printf("Could not establish a connection: maximum number of retries reached\n");
				return ERROR;
			}
			break;
		case RECEIVER:
			if (isCommand(receiveFrame(al->fd), SET)) {
				sendCommand(al->fd, UA);
				printf("Connection successfully established!\n");
			}
			break;
		default:
			break;
	}

	return 0;
}

int llwrite(unsigned char* buf, int bufSize) {
	int counter = 0;
	Frame receivedFrame;

	while(counter < ll->numRetries) {
		if (counter == 0 || alarmFired) {
			alarmFired = 0;
			sendDataFrame(al->fd, buf, bufSize);
			counter++;

			setAlarm();
		}

		receivedFrame = receiveFrame(al->fd);

		if (isCommand(receivedFrame, RR)) {
			ll->statistics.rrSent++;

			if(ll->sn != receivedFrame.sn)
				ll->sn = receivedFrame.sn;

			stopAlarm();
			counter--;
			break;

		} else if (isCommand(receivedFrame, REJ)) {
			ll->statistics.rejSent++;
			counter = 0;
			stopAlarm();
		}

	}

	if (counter >= ll->numRetries) {
		printf("Could not send frame: maximum number of retries reached\n");
		stopAlarm();
		return ERROR;
	}

	return 0;
}

int llread(unsigned char ** message) {
	int disc = 0, dataSize;
	Frame frm;

	while (!disc) {
		frm = receiveFrame(al->fd);
		
		switch (frm.type) {
			case COMMAND:
				if (isCommand(frm, DISC))
					disc = 1;

				break;
			case DATA:
				if (frm.answer == RR && ll->sn == frm.sn) {
					ll->statistics.rrRcvd++;
					ll->sn = !frm.sn;
					dataSize = frm.size - DATA_FRAME_SIZE;
					*message = malloc(dataSize);
					memcpy(*message, &frm.frame[4], dataSize);
					disc = 1;
				}
				else if (frm.answer == REJ) {
					ll->statistics.rejRcvd++;
					ll->sn = frm.sn;
				}

				if (frm.answer != NONE)
					sendCommand(al->fd, frm.answer);
				break;
			case INVALID:
				break;
			default:
				return ERROR;
		}

	}

	return 0;
}

int llclose() {
	int counter = 0;

	switch(al->status) {
		case TRANSMITTER:
			while(counter < ll->numRetries) {
				if (counter == 0 || alarmFired) {
					alarmFired = 0;
					sendCommand(al->fd, DISC);
					counter++;
					setAlarm();
					if (isCommand(receiveFrame(al->fd), DISC)) {
						sendCommand(al->fd, UA);
						sleep(1);
						break;
					}
				}
			}

			stopAlarm();
			if (counter < ll->numRetries)
				printf("Connection successfully terminated!\n");
			else {
				printf("Could not disconect: maximum number of retries reached\n");
				return ERROR;
			}
			break;
		case RECEIVER:
			while (!isCommand(receiveFrame(al->fd), DISC))
				continue;

			while(counter < ll->numRetries) {
				if (counter == 0 || alarmFired) {
					alarmFired = 0;
					counter++;
					setAlarm();
					sendCommand(al->fd, DISC);
					if (isCommand(receiveFrame(al->fd), UA))
						break;
				}
			}
			stopAlarm();
			if (counter < ll->numRetries)
				printf("Connection successfully terminated!\n");
			else {
				printf("Could not disconect: maximum number of retries reached\n");
				return ERROR;
			}
			break;
		default:
			break;
	}

	return 0;
}

int isCommand(Frame frm, Command cmd) {
	if (frm.type == INVALID)
		return 0;

	switch (frm.frame[2] & 0x0F) {
	case C_SET:
		if (cmd == SET)
			return 1;
		else
			return 0;
	case C_UA:
		if (cmd == UA)
			return 1;
		else
			return 0;
	case C_RR:
		if (cmd == RR)
			return 1;
		else
			return 0;
	case C_REJ:
		if (cmd == REJ)
			return 1;
		else
			return 0;
	case C_DISC:
		if (cmd == DISC)
			return 1;
		else
			return 0;
	default:
		return 0;
	}

	return 0;
}

unsigned char getBCC2(unsigned char* data, unsigned int size) {
	unsigned char BCC = 0;

	int i;
	for (i = 0; i < size; i++)
		BCC ^= data[i];

	return BCC;
}

int sendDataFrame(int fd, unsigned char* data, unsigned int size) {
	Frame df;
	df.size =  size + DATA_FRAME_SIZE;
	
	df.frame[0] = FLAG;
	df.frame[1] = A03;
	df.frame[2] = ll->sn << 5;
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

int sendCommand(int fd, Command cmd) {
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
		case RR:
			frame[1] = getAFromRspn();
			frame[2] = C_RR;
			frame[2] |= (ll->sn << 5);
			frame[3] = frame[1] ^ frame[2];
			break;
		case REJ:
			frame[1] = getAFromRspn();
			frame[2] = C_REJ;
			frame[2] |= (ll->sn << 5);
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

Frame receiveFrame(int fd) {
	unsigned char c;
	int res, receiving = 1, state = 0, dataFrame = 0, i = 0;
	Frame frm;
	frm.type = INVALID;

	while(receiving) {
		res = read(fd, &c, 1);
		
		if (res < 1)
			return frm;

		switch(state) {
		case 0:
			if (c == FLAG) {
				frm.frame[i] = c;
				i++;
				state++;
			}
			break;
		case 1:
			if (c == A01 || c == A03) {
				frm.frame[i] = c;
				i++;
				state++;
			}
			else if (c != FLAG) {
				state = 0;
				i = 0;
			}
			break;
		case 2: 
			if (c != FLAG) {
				frm.frame[i] = c;
				i++;
				state++;
			}
			else if (c == FLAG) {
				state = 1;
				i = 1;
			}
			else {
				state = 0;
				i = 0;
			}
			break;
		case 3:
			if (c == (frm.frame[1]^frm.frame[2])) {
				frm.frame[i] = c;
				i++;
				state++;
			}
			else if (c == FLAG) {
				state = 1;
				i = 1;
			}
			else {
				state = 0;
				i = 0;
			}
			break;
		case 4:
			if (c == FLAG) {
				frm.frame[i] = c;
				i++;
				frm.frame[i] = 0;
				receiving = 0;

				if (i > 5)
					dataFrame = 1;
			}
			else {
				frm.frame[i] = c;
				i++;
			}
			break;
		default:
			break;
		}
	}

	if (dataFrame) {
		frm.size = i;
		frm.answer = NONE;
		frm.type = DATA;
		frm = destuff(frm);
		
		// check BCC1
		if (frm.frame[3] != (frm.frame[1] ^ frm.frame[2])) {
			printf("ERROR in receiveFrame(): BCC1 error\n");
			return frm;
		}
		
		// check BCC2
		int dataSize = frm.size - DATA_FRAME_SIZE;
		unsigned char BCC2 = getBCC2(&frm.frame[4], dataSize);
		if (frm.frame[4 + dataSize] != BCC2) {			
			printf("ERROR in receiveFrame(): BCC2 error\n");
			frm.answer = REJ;
		}

		if (frm.answer == NONE)
			frm.answer = RR;

		frm.sn = (frm.frame[2] >> 5) & BIT(0);
	}
	else {
		frm.type = COMMAND;
		if (isCommand(frm, RR) || isCommand(frm, REJ))
			frm.sn = (frm.frame[2] >> 5) & BIT(0);
	}
	return frm;
}

Frame stuff(Frame df) {
	Frame stuffedFrame;
	unsigned int newSize = df.size;

	int i;
	for (i = 1; i < (df.size - 1); i++) {
		if (df.frame[i] == FLAG || df.frame[i] == ESCAPE)
			newSize++;
	}

	stuffedFrame.frame[0] = df.frame[0];
	int j = 1;
	for (i = 1; i < (df.size - 1); i++) {
		if (df.frame[i] == FLAG || df.frame[i] == ESCAPE) {
			stuffedFrame.frame[j] = ESCAPE;
			stuffedFrame.frame[++j] = df.frame[i] ^ 0x20;
		}
		else
			stuffedFrame.frame[j] = df.frame[i];
		j++;
	}
	
	stuffedFrame.frame[j] = df.frame[i];
	stuffedFrame.size = newSize;
	
	return stuffedFrame;
}

Frame destuff(Frame df) {
	Frame destuffedFrame;
	destuffedFrame.sn = df.sn;
	destuffedFrame.type = df.type;
	destuffedFrame.answer = df.answer;
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
