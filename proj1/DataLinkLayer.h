#pragma once

#include <termios.h>

#include "Utilities.h"

#define C_SET 0x03
#define C_UA 0x07
#define C_RR 0x05
#define C_REJ 0x01
#define C_DISC 0x0B

#define FLAG 0x7E
#define A03 0x03
#define A01 0x01
#define ESCAPE 0x7D

typedef enum {
	SET, UA, RR, REJ, DISC
} Frame;

typedef struct {
	char port[20];
	int baudRate;
	unsigned int sequenceNumber;
	unsigned int timeout;
	unsigned int numRetries;
	char frame[MAX_FRAME_SIZE];
	struct termios oldtio, newtio;
} LinkLayer;

typedef struct {
	char frame[MAX_FRAME_SIZE];
	unsigned int size;
} DataFrame;

extern LinkLayer* ll;

int initLinkLayer(char* port, int baudRate, unsigned int timeout, unsigned int numTransmissions);

int openSerialPort(char* port);

int closeSerialPort();

int setNewTermios();

int llopen();

int llclose();

unsigned char getBCC2(unsigned char* data, unsigned int size);

int sendDataFrame(int fd, unsigned char* data, unsigned int size);

int sendFrame(int fd, Frame frame);

unsigned char getAFromCmd();

unsigned char getAFromRspn();

int receiveFrame(int fd);

DataFrame stuff(DataFrame df);

DataFrame destuff(DataFrame df);

int stateMachine(unsigned char c, int state, char cmd[]);