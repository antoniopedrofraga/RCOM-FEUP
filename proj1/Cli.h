#pragma once

int getMode();

void clrscr();

char * getPort();

char * getFileName(int status);

int getRetries();

int getTimeout();

void printProgressBar(char * fileName, int bytes, int size, int status);

void printWaiting(int status);
