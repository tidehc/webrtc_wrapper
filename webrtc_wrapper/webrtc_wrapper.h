// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the WEBRTC_WRAPPER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// WEBRTC_WRAPPER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WEBRTC_WRAPPER_EXPORTS
#define WEBRTC_WRAPPER_API __declspec(dllexport)
#else
#define WEBRTC_WRAPPER_API __declspec(dllimport)
#endif

// This class is exported from the webrtc_wrapper.dll
class WEBRTC_WRAPPER_API Cwebrtc_wrapper {
public:
	Cwebrtc_wrapper(void);
	// TODO: add your methods here.
};

extern WEBRTC_WRAPPER_API int nwebrtc_wrapper;

WEBRTC_WRAPPER_API int fnwebrtc_wrapper(void);
