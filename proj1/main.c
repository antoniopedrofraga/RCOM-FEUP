#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "Utilities.h"
#include "ApplicationLayer.h"

int main(int argc, char** argv)
{
    
	if ( (argc != 3) || 
		((strcmp("/dev/ttyS0", argv[1])!=0) && 
			(strcmp("/dev/ttyS1", argv[1])!=0) &&
			(strcmp("/dev/ttyS4", argv[1])!=0)) ||
		((strcmp("RECEIVER", argv[2]) != 0)
			&& (strcmp("TRANSMITTER", argv[2]) != 0))) {
		printf("Usage:\tnserial SerialPort flag\n\tex: nserial /dev/ttyS1 TRANSMITTER/RECEIVER\n");
	exit(ERROR);
}
    
    int mode;
    if((strcmp("RECEIVER", argv[2]) == 0)){
	mode = RECEIVER;
    }else{
	mode = TRANSMITTER;
    }

    initAppLayer(argv[1], mode);
    return 0;
}
