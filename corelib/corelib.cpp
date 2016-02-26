// corelib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "core_instance.h"

#ifndef _WIN32
extern "C"
#endif
bool get_core_instance(core::icore_interface** _core)
{
	*_core = new core::core_instance();

	return true;
}

