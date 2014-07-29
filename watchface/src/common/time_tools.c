#include <pebble.h>
#include "time_tools.h"

char* weekday_tostring(char *buf, int weekday) {
	switch(weekday) {
		case MONDAY:
			strcpy(buf, "Måndag");
			break;
		case TUESDAY:
			strcpy(buf, "Tisdag");
			break;
		case WEDNESDAY:
			strcpy(buf, "Onsdag");
			break;
		case THURSDAY:
			strcpy(buf, "Torsdag");
			break;
		case FRIDAY:
			strcpy(buf, "Fredag");
			break;
		case SATURDAY:
			strcpy(buf, "Lördag");
			break;
		case SUNDAY:
			strcpy(buf, "Söndag");
			break;
	}

	return buf;
}