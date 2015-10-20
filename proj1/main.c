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

#define RECEIVER 0
#define WRITER 1

#define ERROR -1

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7e
#define A 0x03
#define C 0x03

volatile int STOP=FALSE;

int flag = 1, counter = 0;

int stateMachine(unsigned char c, int state,char temp[])
{
		switch(state){
			case 0:
				if(c == FLAG)
				{
					temp[state] = c;
					state++;
				}
				break;
			case 1: 
				if(c == A)
				{
					temp[state] = c;
					state++;
				}
				else if(c == FLAG)
						state = 1;
					else
						state = 0;
				break;
			case 2: 
				if(c == C)
				{
					temp[state] = c;
					state++;
				}
				else if(c == FLAG)
						state = 1;
					else
						state = 0;
				break;
			case 3:
				if(c == (temp[1]^temp[2]))
				{
					temp[state] = c;
					state++;
				}
				else if(c == FLAG)
						state = 1;
					else
						state = 0;
				break;
			case 4:
				if(c == FLAG)
				{
					temp[state] = c;
					STOP = TRUE;
				}
				else
					state = 0;
				break;
		}
	return state;	
}


int main(int argc, char** argv)
{
    int fd,res;
    struct termios oldtio,newtio;
    char buf[255];
    
    if ( (argc != 3) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) &&
		  (strcmp("/dev/ttyS4", argv[1])!=0)) ||
		((strcmp("RECEIVER", argv[2]) != 0)
		&& (strcmp("WRITER", argv[2]) != 0))) {
      printf("Usage:\tnserial SerialPort flag\n\tex: nserial /dev/ttyS1 WRITER/RECEIVER\n");
      exit(1);
    }
    
    int mode;
    if((strcmp("RECEIVER", argv[2]) == 0)){
	mode = RECEIVER;
    }else{
	mode = WRITER;
    }

    fd = openSerialPort(argv[1], &oldtio, &newtio);
    if(fd < 0){
	printf("Error opening serial port");
	exit(ERROR);
	}
    
    llopen(mode, &fd);

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}



int openSerialPort(char*serialPort, struct termios*oldtio, struct termios*newtio){

    int fd = open(serialPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {perror(serialPort); return ERROR; }

    if ( tcgetattr(fd,oldtio) == ERROR) { /* save current port settings */
      perror("tcgetattr");
      return ERROR;
    }
    
    bzero(newtio, sizeof(*newtio));
    (*newtio).c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    (*newtio).c_iflag = IGNPAR;
    (*newtio).c_oflag = 0;

 
    (*newtio).c_lflag = 0;

    (*newtio).c_cc[VTIME]    = 0;   
    (*newtio).c_cc[VMIN]     = 1;   


    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,newtio) == -1) {
      perror("tcsetattr");
      return ERROR;
    }

    printf("New termios structure set\n");
    return fd;
}



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
	int state = 0;
	
	while(STOP == FALSE && !flag)
	{
		res = read(*fd, buf, 1);
		if (res > 0) {
			state = stateMachine(buf[0],state,tmp);
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


int llopen(int mode, int*fd){
    int res, counter, flag;
    char buf[1];
    char tmp[5];
    int i = 0;
    int state = 0;
    unsigned char UA[5]; 
    switch(mode){
	case WRITER:
		(void) signal(SIGALRM, incrementCounter);

		while(counter < 3) {
			if (flag) {
				alarm(3);
				printf("Sending message\n");
				setAndWrite(fd);
				flag = 0;
				if (getResponse(fd) == 0) {
					printf("Success!\n");
					break;
				}
			}
		}
	break;
	case RECEIVER:
    		while (STOP==FALSE) {
      			res = read(*fd, buf, 1);
			if (res > 0)
				state = stateMachine(buf[0],state,tmp);
    		}

		UA[0] = FLAG;
		UA[1] = A;
		UA[2] = C;
		UA[3] = A^C;
		UA[4] = FLAG;

		if(tmp[3] != (tmp[1]^tmp[2])) {
			printf("Error!");
			exit(1);
		}else{
			printf("It's ok, %x, %x, %x, %x, %x\n", tmp[0], tmp[1], tmp[2],tmp[3],tmp[4]);
		}	

		tcflush(*fd, TCOFLUSH);
		sleep(1);
		res = write(*fd, UA, 5);
		sleep(1);
	break;
	default:
	return ERROR;
    }
	
}

