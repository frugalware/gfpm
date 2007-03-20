#ifndef __GFPM_MESSAGES_H__
#define __GFPM_MESSAGES_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

typedef enum _gfpmerrortype
{
	GFPM_ERROR_STDOUT = 1, 	/* Display error on stdout */
	GFPM_ERROR_GUI 		/* Display error on gui */
} GfpmErrorType;

void gfpm_error (const char *, GfpmErrorType);

void gfpm_message (const char *);
	
#endif
