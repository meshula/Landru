
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "RaiseError.h"
#include <stdio.h>

static int errorRaised = 0;

int CurrentError() 
{ 
	return errorRaised;
}

void RaiseError(int pc, const char* msg, const char* aux)
{
	if (aux == 0)
		aux = "";
	printf("[%d] %s: %s\n", pc, msg, aux);
	++errorRaised;
}
