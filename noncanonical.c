/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7e
#define A 0x03
#define C 0x07

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[1];
	char tmp[5];
	unsigned char UA[5]; 

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
      
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    tcgetattr(fd,&oldtio); /* save current port settings */

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 char received */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

    printf("New termios structure set\n");

	int i = 0;
    while (STOP==FALSE) {
      	res = read(fd, buf, 1);
		tmp[i] = buf[0];
		if(tmp[i] == FLAG && i!=0)
			STOP = TRUE;	
		else
			i++;
    }

	UA[0] = FLAG;
	UA[1] = A;
	UA[2] = C;
	UA[3] = A^C;
	UA[4] = FLAG;

	if(tmp[3] != (tmp[1]^tmp[2])) {
			printf("Error!");
			exit(1);
		}

	//printf("%x, %x, %x, %x, %x\n", temp[0], temp[1], temp[2],temp[3],temp[4]);	

	tcflush(fd, TCOFLUSH);
	sleep(1);
	res = write(fd, UA, 5);
	sleep(1);

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
