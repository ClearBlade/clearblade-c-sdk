#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "die.h"
#include "cb_log.h"

/**
  * Function to stop execution of the program if things go bad gonna go down!
*/
void die(const char *message) {
	if(errno) {
        perror(message);
    } else {
        log_error("C SDK - %s\n", message);
    }

    exit(1);
}