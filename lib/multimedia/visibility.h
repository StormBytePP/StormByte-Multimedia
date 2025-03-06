#pragma once

#include <definitions.h>

#ifdef WINDOWS
	#ifdef StormByte_Config_EXPORTS
		#define STORMBYTE_CONFIG_PUBLIC	__declspec(dllexport)
  	#else
      	#define STORMBYTE_CONFIG_PUBLIC	__declspec(dllimport)
  	#endif
  	#define STORMBYTE_CONFIG_PRIVATE
#else
    #define STORMBYTE_CONFIG_PUBLIC		__attribute__ ((visibility ("default")))
    #define STORMBYTE_CONFIG_PRIVATE	__attribute__ ((visibility ("hidden")))
#endif
