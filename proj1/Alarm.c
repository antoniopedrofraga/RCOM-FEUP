#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "DataLinkLayer.h"

int alarmFired = 0;

void handler(int signal) {
	if (signal != SIGALRM)
		return;

	alarmFired = 1;

	printf("Timeout!\n");
}

void setAlarm() {
	struct sigaction action;
	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	sigaction(SIGALRM, &action, NULL);

	alarmFired = 0;

	alarm(ll->timeout);
}

void stopAlarm() {
	struct sigaction action;
	action.sa_handler = NULL;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	sigaction(SIGALRM, &action, NULL);

	alarm(0);
}