#pragma once

int getMode();

void clrscr();

char * getPort();

char * getFileName(int mode);

int getRetries();

int getTimeout();

void printProgressBar(char * fileName, int bytes, int size, int mode);
