
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LAB_RAISEERROR_H
#define LAB_RAISEERROR_H

#ifndef EXTERNC
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif
#endif

EXTERNC void RaiseError(int pc, const char* msg, const char* aux);
EXTERNC int CurrentError();


#endif
