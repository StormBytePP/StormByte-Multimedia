#pragma once

#include <StormByte/platform.h>

#ifdef WINDOWS
	#ifdef StormByte_Multimedia_EXPORTS
		#define STORMBYTE_MULTIMEDIA_PUBLIC	__declspec(dllexport)
  	#else
      	#define STORMBYTE_MULTIMEDIA_PUBLIC	__declspec(dllimport)
  	#endif
  	#define STORMBYTE_MULTIMEDIA_PRIVATE
#else
    #define STORMBYTE_MULTIMEDIA_PUBLIC		__attribute__ ((visibility ("default")))
    #define STORMBYTE_MULTIMEDIA_PRIVATE	__attribute__ ((visibility ("hidden")))
#endif

#ifdef ENABLE_ADVANCED
	#define STORMBYTE_MULTIMEDIA_ADVANCED	STORMBYTE_MULTIMEDIA_PUBLIC
#else
	#define STORMBYTE_MULTIMEDIA_ADVANCED	STORMBYTE_MULTIMEDIA_PRIVATE
#endif