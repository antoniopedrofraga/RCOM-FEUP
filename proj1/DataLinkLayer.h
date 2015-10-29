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
	SET, UA, RR, REJ, DISC, NONE
} Command;

typedef enum {
	INVALID, DATA, COMMAND
} FrameType;

typedef struct {
	int timeout;

	int msgSent;
	int msgRcvd;

	int rrSent;
	int rrRcvd;

	int rejSent;
	int rejRcvd;

} Statistics;

typedef struct {
	char port[20];
	int baudRate;
	unsigned int sn;
	unsigned int timeout;
	unsigned int numRetries;
	unsigned int pkgSize;
	struct termios oldtio, newtio;
	Statistics statistics;
} LinkLayer;

typedef struct {
	unsigned char frame[MAX_FRAME_SIZE];
	unsigned int size;
	unsigned int sn;
	FrameType type;
	Command answer;
} Frame;


extern LinkLayer* ll;

int initLinkLayer(char* port, int baudRate, int pkgSize, unsigned int timeout, unsigned int numTransmissions);

int getBaudrateChoice(int choice);

int openSerialPort(char* port);

int closeSerialPort();

int setNewTermios();

int llopen();

int llclose();

int llwrite(unsigned char * buf, int bufSize);

int llread(unsigned char ** message);

unsigned char getBCC2(unsigned char* data, unsigned int size);

int sendDataFrame(int fd, unsigned char* data, unsigned int size);

int sendCommand(int fd, Command cmd);

int isCommand(Frame frm, Command cmd);

unsigned char getAFromCmd();

unsigned char getAFromRspn();

Frame receiveFrame(int fd);

Frame stuff(Frame df);

Frame destuff(Frame df);
