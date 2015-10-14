/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7e
#define A 0x03
#define C 0x03

volatile int STOP = FALSE;
int flag = 1, counter = 0;

void incrementCounter()
{
	counter++;
	flag = 1;
	printf("Timeout!\n");
}

void setAndWrite(int *fd)
{
	tcflush(*fd, TCOFLUSH);
	unsigned char SET[5];
	unsigned char BCC;
	SET[0] = FLAG;
	SET[1] = A;
	SET[2] = C;
	BCC = SET[1] ^ SET[2];
	SET[3] = BCC;
	SET[4] = FLAG;
	write(*fd, SET, 5);
}

int getResponse(int *fd)
{
	char buf[5];
	char tmp[5];
	int res;
	tcflush(*fd, TCIFLUSH);
	int i = 0;

	while(STOP == FALSE && !flag)
	{
		res = read(*fd, buf, 1);
		if (res > 0) {
			tmp[i] = buf[0];
			if (tmp[i] == FLAG && i!= 0) {
				STOP = TRUE;
			}
			else
				i++;
		}
		
	}
	
	if (flag)
		return 1;

	if (tmp[3] != (tmp[1]^tmp[2])) {
		printf("Error\n");
		return 1;
	}

	return 0;
}

int main(int argc, char** argv)
{
    int fd,res;
    struct termios oldtio,newtio;
    char buf[255];
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) &&
		  (strcmp("/dev/ttyS4", argv[1])!=0))) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 char received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

	(void) signal(SIGALRM, incrementCounter);
	while(counter < 3) {
		if (flag) {
			alarm(3);
			printf("Sending message\n");
			setAndWrite(&fd);
			flag = 0;
			if (getResponse(&fd) == 0) {
				printf("Success!\n");
				break;
			}
		}
	}
 	

  /* 
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar 
    o indicado no guião 
  */
   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }


    close(fd);
    return 0;
}
