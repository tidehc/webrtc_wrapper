#include "common_.hpp"

// #include "common.h"
#include "encrypt_ctx_.hpp"

#if WE_UNDER_WINDOWS
#	include <windows.h>
#	include <comutil.h>
#	include <shlwapi.h>
#	include <shlobj.h>
#	include <AtlConv.h>
#	include "webrtc/base/win32socketinit.h"
#	include "webrtc/base/win32socketserver.h"
//#	include "resource.h"
# elif WE_UNDER_MAC
#	include "webrtc/base/maccocoasocketserver.h"
#   import <CoreFoundation/CoreFoundation.h>
#endif

#ifdef _MSC_VER
#pragma comment(lib,"shlwapi.lib")
#endif

#include "webrtc/base/ssladapter.h"
#include "webrtc/base/thread.h"
#include "webrtc/base/json.h"

static bool g_bInitialized = false;
#if WE_UNDER_WINDOWS
static bool g_winCoInitialize = false;
#endif

rtc::Thread* s_InitThread = nullptr;
cpp11::shared_ptr<_EncryptCtx> s_encrypt_ctx = nullPtr;

WEBRTC_WRAPPER_API int libwebrtc_initialize()
{
	if (!g_bInitialized) {
		s_InitThread = rtc::Thread::Current();
#if 0
		StartDebug();
#endif

#if WE_UNDER_WINDOWS
		HRESULT hr = E_FAIL; // CoInitializeEx(NULL, COINIT_MULTITHREADED);
		g_winCoInitialize = SUCCEEDED(hr);
		rtc::EnsureWinsockInit();
#if 1
		static rtc::Win32Thread w32_thread;
		rtc::ThreadManager::Instance()->SetCurrentThread(&w32_thread);
#endif
#endif

#if WE_UNDER_MAC
		static rtc::MacCocoaSocketServer ss;
		static rtc::SocketServerScope ss_scope(&ss);
#endif

		rtc::InitializeSSL();
		rtc::InitializeSSLThread();

		s_encrypt_ctx = cpp11::shared_ptr<_EncryptCtx>(_EncryptCtx::New());

		g_bInitialized = true;
	}
#if 0
	if (InitializeAdditionals) {
		return InitializeAdditionals();
	}
#endif

	return EXIT_SUCCESS;
}

WEBRTC_WRAPPER_API void libwebrtc_deinitialize()
{
	if (g_bInitialized) {
		rtc::CleanupSSL();
		g_bInitialized = false;
#if WE_UNDER_WINDOWS
		if (g_winCoInitialize) {
			CoUninitialize();
		}
#endif
		s_InitThread = nullptr;
	}
}

// Proxy to webrtc-everywhere 
WEBRTC_WRAPPER_API void libeverywhere_take_fake_peer_connection_factory()
{
	// Lazy man's wrapper, call webrtc-everywhere's function call
	::TakeFakePeerConnectionFactory();
}

WEBRTC_WRAPPER_API void libeverywhere_release_fake_peer_connection_factory()
{
	// Lazy man's wrapper, call webrtc-everywhere's function call
	::ReleaseFakePeerConnectionFactory();
}
