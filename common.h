#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "eibclient.h"

struct Item {
    char* name;
    eibaddr_t address;
};
typedef struct Item item;

struct Item* item_registry;
int item_registry_size;

void error(const char *msg, ...);

void add_item(item i);

item get_item(int pos);

int lookup_item(eibaddr_t address);

eibaddr_t knx_convert_gaddr(const char *addr);

void knx_send(char* url, eibaddr_t group, int state);

int knx_read(char* url, eibaddr_t EIBOpenT_Group_async);

void knx_watch(char* url, eibaddr_t *group, int *value);