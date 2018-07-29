#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "eibclient.h"

void error(const char *msg, ...);

eibaddr_t knx_convert_gaddr(const char *addr);

void knx_send(char* url, char* group, int state);

int knx_read(char* url, char* group);

int knx_watch(char* url, char* group);