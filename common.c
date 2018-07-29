#include "common.h"

/*
 *  error: 
 *  display an error message using printf syntax and exit 
 */
void error(const char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	if (errno)
		fprintf(stderr, ": %s\n", strerror(errno));
	else
		putc('\n', stderr);
	exit(1);
}