#pragma once

int getMode();

void clrscr();

char * getPort();

char * getFileName(int status);

int getRetries();

int getTimeout();

int getBaudrate();

void printProgressBar(char * fileName, int bytes, int size, int status);

void printWaiting(int status);

int getPktSize();
