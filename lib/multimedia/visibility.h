#pragma once

#include <definitions.h>

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
