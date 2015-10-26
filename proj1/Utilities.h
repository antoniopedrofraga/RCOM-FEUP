#pragma once

#define TRANSMITTER 0
#define RECEIVER 1
#define BAUDRATE B38400
#define ERROR -1

#define FRAME_SIZE 5
#define DATA_FRAME_SIZE 6
#define MAX_BUF_SIZE 256
#define MAX_FRAME_SIZE (MAX_BUF_SIZE + DATA_FRAME_SIZE)

#define MAX_STR_SIZE 100

#define BIT(n) (0x01 << n)

typedef enum {
	CTRL_PKG_DATA = 0, CTRL_PKG_START = 1, CTRL_PKG_END = 2
} ControlPackageType;