#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "die.h"

/**
  * Function to stop execution of the program if things go bad gonna go down!
*/
void die(const char *message) {
	if(errno) {
        perror(message);
    } else {
        printf("ERROR: %s\n", message);
    }

    exit(1);
}