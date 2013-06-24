
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LC_RAISEERROR_H
#define LC_RAISEERROR_H

#ifndef EXTERNC
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif
#endif

EXTERNC void lcRaiseError(const char* msg, const char* aux, int n);
EXTERNC int lcCurrentError();


#endif
