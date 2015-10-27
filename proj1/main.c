#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "Utilities.h"
#include "ApplicationLayer.h"
#include "Cli.h"


int main(int argc, char** argv)
{
    
	if (argc > 1) {
		clrscr();
		printf("ERROR! This programs takes no arguments\n");
		exit(ERROR);
	}

	int mode = getMode();

 	if (mode == ERROR )
		return ERROR;
	else if( mode == RECEIVER || mode == TRANSMITTER)
		;
	else	
		return ERROR;	
	
	char * port = getPort();

	int retries = getRetries();

	int timeout = getTimeout();

    	char * fileName = getFileName(mode);

    	initAppLayer(port, mode, fileName, timeout, retries);

   	return 0;
}

