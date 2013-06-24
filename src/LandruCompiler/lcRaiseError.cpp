
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruCompiler/lcRaiseError.h"
#include <stdio.h>
#include <string.h>

static int errorRaised = 0;

int lcCurrentError() 
{ 
	return errorRaised;
}

void lcRaiseError(const char* msg, const char* aux, int n)
{
    if (aux == 0)
        aux = "";
    
    char buff[1024];
    strncpy(buff, aux, n);
    buff[n] = '\0';
    
	printf("\n<--- %s: %s--->\n", msg, buff);
	++errorRaised;
}
