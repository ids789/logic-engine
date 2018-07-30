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

/*
 *  add_item: append an item to the items_registry
 */
void add_item(item i) {
	item_registry_size++;
	item_registry = realloc(item_registry, item_registry_size);
	item_registry[item_registry_size-1] = i;
}

/*
 *  get_item: get an item from the items_registry
 */
item get_item(int pos) {
	if (pos >= item_registry_size)
		error("invalid item pos");
	return item_registry[pos];
}

/*
 *  lookup_item: find an items position using it's address
 */
int lookup_item(eibaddr_t address) {
	for (int i = 0; i < item_registry_size; i++) {
		if (item_registry[i].address == address)
			return i;
	}
	return -1;
}
