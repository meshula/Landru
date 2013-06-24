
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRUEXCEPTION_H
#define LANDRUEXCEPTION_H

#define HAVE_EXCEPTION_HANDLING
#ifdef HAVE_EXCEPTION_HANDLING
	#define LANDRU_THROW(a) throw(a)
#else
	#define LANDRU_THROW(a) assert(false);
#endif

#endif
