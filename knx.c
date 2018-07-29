#include "common.h"

/* 
 *  knx_convert_gaddr: convert a group from a string to binary value
 *  - group values must be in the form X/X/X
 */ 
eibaddr_t knx_convert_gaddr(const char *addr) {
	unsigned int a, b, c, res;
	res = sscanf(addr, "%u/%u/%u", &a, &b, &c);
	
	if (res == 3 && a <= 0x1F && b <= 0x07 && c <= 0xFF)
		return (a << 11) | (b << 8) | c;
	if (res == 2 && a <= 0x1F && b <= 0x7FF)
		return (a << 11) | (b & 0x7FF);
	if (sscanf(addr, "%x", &a) == 1 && a <= 0xFFFF)
		return a;
	
	error("invalid group address format %s", addr);
	return 0;
}

/*
 *  knx_send: send a telegram to a group
 *  - currently only supporting groups with a binary datatype
 *  - state is: 0 = off, 1 = on
 */
void knx_send(char* url, char* group, int state) {
	int len;
	eibaddr_t dest;
	EIBConnection *con;
	unsigned char buf[2];

	con = EIBSocketURL(url);
	if (!con)
		error("KNX Group Socket Open failed");
	
	dest = knx_convert_gaddr(group);
   
    // setup the knx telegram payload
	buf[0] = 0;
	buf[1] = 0x80 + state;

	if (EIBOpenT_Group(con, dest, 1) == -1)
		error("KNX Group Socket Connect failed");

	len = EIBSendAPDU(con, 2, buf);
	if (len == -1)
		error("KNX Request failed");

	EIBClose(con);
}

/*
 *  knx_read: retrieve a knx group's current value
 *  - currently only supports binary datatypes
 *  - return value is: 0 = off, 1 = on
 *  TODO: add a timeout using EIB_Poll_FD
 */
int knx_read(char* url, char* group) {
	int result;
	int len;
	eibaddr_t dest, src;
	EIBConnection *con;
	unsigned char buf[200];
	unsigned char req_buf[2] = { 0, 0 };

    con = EIBSocketURL(url);
	if (!con)
		error("KNX Group Socket Open failed");
	
	dest = knx_convert_gaddr(group);

    if (EIBOpenT_Group(con, dest, 0) == -1)
		error("KNX Group Socket Connect failed");

    len = EIBSendAPDU(con, 2, req_buf);
    if (len == -1)
		error("KNX Request failed");
    
	while (1) {
	  	len = EIBGetAPDU_Src(con, sizeof(buf), buf, &src);

		if (len == -1 || len < 2)
	    	error("KNX Read failed");

		// Ensure the APDU is known
	  	if (buf[0] & 0x3 || (buf[1] & 0xC0) == 0xC0)
			continue;

		// Ensure the telegram was a response or write
		if (((buf[1] & 0xC0) == 0x40) || 
			((buf[1] & 0xC0) == 0x80)) {
			if (len == 2) {
				result = buf[1] & 0x3F;
				break;
			}
		}
	}

	EIBClose(con);
	return result;
}

/*
 *  knx_watch: listen for events on the knx bus
 *  - returns all telegrams sent over the knx bus with binary values
 *  - group names are put into the group buffer and the value is returned
 */
int knx_watch(char* url, char* group) {
	int value;
	int len;
	eibaddr_t dest, src;
	EIBConnection *con;
	unsigned char buf[200];

	con = EIBSocketURL(url);
	if (!con)
		error("KNX Group Socket Open failed");

	if (EIBOpen_GroupSocket(con, 0) == -1)
		error("Connect failed");

	while (1) {
		len = EIBGetGroup_Src(con, sizeof (buf),
							  (unsigned char*)buf, &src, &dest);

		if (len == -1 || len < 2)
	    	error("KNX Read failed");

		// Ensure the APDU is known
	  	if (buf[0] & 0x3 || (buf[1] & 0xC0) == 0xC0)
			continue;

		// Ensure the telegram was a response or write
		if (((buf[1] & 0xC0) == 0x40) || 
			((buf[1] & 0xC0) == 0x80)) {
			if (len == 2) {
				value = buf[1] & 0x3F;
				break;
			}
		}
	}

	sprintf(group, "%d/%d/%d", (dest >> 11) & 0x1f,
			(dest >> 8) & 0x07, (dest) & 0xff);
	
	EIBClose(con);
	return value;
}