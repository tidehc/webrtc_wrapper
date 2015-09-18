// webrtc_wrapper.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "webrtc_wrapper.h"


// This is an example of an exported variable
WEBRTC_WRAPPER_API int nwebrtc_wrapper=0;

// This is an example of an exported function.
WEBRTC_WRAPPER_API int fnwebrtc_wrapper(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see webrtc_wrapper.h for the class definition
Cwebrtc_wrapper::Cwebrtc_wrapper()
{
	return;
}
