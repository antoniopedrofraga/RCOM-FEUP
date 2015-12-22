#include <stdio.h>

void debug_msg (int debug_mode, char * msg) {
	if (debug_mode) {
		printf(msg, "Error printing msg, missing argument!");
		printf("\n");
	}
}


void debug_sub_msg (int debug_mode, char * msg) {
	if (debug_mode) {
		printf("\t->");
		printf(msg, "\t->Error printing msg, missing argument!");
		printf("\n");
	}
}