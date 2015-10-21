#pragma once

#include <termios.h>

#include "Utilities.h"

#define C_SET 0x03
#define C_UA 0x07
#define C_RR 0x05
#define C_REJ 0x01
#define C_DISC 0x0B

#define FLAG 0x7E
#define A 0x03
#define ESCAPE 0x7D

typedef enum {
	SET, UA, RR, REJ, DISC
} Command;

typedef struct {
	char port[20];
	int baudRate;
	unsigned int sequenceNumber;
	unsigned int timeout;
	unsigned int numRetries;
	char frame[MAX_SIZE];
	struct termios oldtio, newtio;
} LinkLayer;

extern LinkLayer* ll;

int initLinkLayer(char* port, int baudRate, unsigned int timeout, unsigned int numTransmissions);

int openSerialPort(char* port);

int closeSerialPort();

int setNewTermios();

int llopen(int mode);

int llclose(int mode);

int sendCommand(int fd, Command cmd);

int receiveCommand(int fd);

int stateMachine(unsigned char c, int state, char cmd[]);