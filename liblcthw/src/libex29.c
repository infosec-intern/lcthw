#include <stdio.h>
#include <ctype.h>
#include "dbg.h"

#define MAX_STRING 20

int print_a_message(const char *msg)
{
	printf("STRING: %s\n", msg);
	return 0;
}

int uppercase(const char *msg)
{
	int i = 0;

	for (i = 0; msg[i] != '\0' && i < MAX_STRING; i++)
		printf("%c", toupper(msg[i]));
	
	printf("\n");
	return 0;
}

int lowercase(const char *msg)
{
	int i = 0;

	for (i = 0; msg[i] != '\0' && i < MAX_STRING; i++)
		printf("%c", tolower(msg[i]));

	printf("\n");
	return 0;
}

int fail_on_purpose(void)
{
	return 1;
}
