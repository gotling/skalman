#pragma once

typedef enum {
	SUNDAY,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY
} DayNames;

char* weekday_tostring(char *buf, int weekday);