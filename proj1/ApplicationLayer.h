#pragma once

typedef struct {
	int fd;
	int status;
} ApplicationLayer;

extern ApplicationLayer* al;

int initAppLayer(char* port, int status);