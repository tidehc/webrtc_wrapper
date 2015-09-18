/* Copyright(C) 2015 Toppano <https://github.com/sukinull/libwebrtc> */
#ifndef _WEBRTC_WRAPPER_COMMON_H_
#define _WEBRTC_WRAPPER_COMMON_H_
#ifdef __cplusplus
extern "C" {
#endif

// Windows
#if defined(WIN32)|| defined(_WIN32) || defined(_WIN32_WCE)
#	define WE_UNDER_WINDOWS	1
#else
typedef int64_t LONGLONG;
typedef unsigned int UINT;
typedef long LRESULT;
typedef void* WPARAM;
typedef void* LPARAM;
typedef void* HWND;
#define CALLBACK
#endif

// OS X or iOS
#if defined(__APPLE__)
#	define WE_UNDER_APPLE				1
#endif
#if TARGET_OS_MAC
#	define WE_UNDER_MAC					1
#endif
#if TARGET_OS_IPHONE
#	define WE_UNDER_IPHONE				1
#endif
#if TARGET_IPHONE_SIMULATOR
#	define WE_UNDER_IPHONE_SIMULATOR	1
#endif


#ifdef _MSC_VER
#	if !defined(_CRT_SECURE_NO_WARNINGS)
#		define _CRT_SECURE_NO_WARNINGS
#	endif
#	define WE_INLINE	_inline
#	define we_stricmp	_stricmp
#	ifndef __STDC_LIMIT_MACROS
#		define __STDC_LIMIT_MACROS
#	endif
#else
#	define WE_INLINE	inline
#	define we_stricmp	stricmp
#endif

#if !defined(__GNUC__) && defined(WEBRTC_WRAPPER_EXPORTS)
# 	define WEBRTC_WRAPPER_API		__declspec(dllexport)
#	define WEBRTC_WRAPPER_GEXTERN	extern __declspec(dllexport)
#elif !defined(__GNUC__) && !defined(WEBRTC_WRAPPER_IMPORTS_IGNORE)
# 	define WEBRTC_WRAPPER_API		__declspec(dllimport)
#	define WEBRTC_WRAPPER_GEXTERN	__declspec(dllimport)
#else
#	define WEBRTC_WRAPPER_API
#	define WEBRTC_WRAPPER_GEXTERN	extern
#endif

static const char kAudioLabel[] = "audio_label";
static const char kVideoLabel[] = "video_label";
static const char kStreamLabel[] = "stream_label";

// http://www.w3.org/TR/webrtc/#idl-def-RTCSignalingState
static const char kSignalingStateStable[] = "stable";
static const char kSignalingStateHaveLocalOffer[] = "have-local-offer";
static const char kSignalingStateHaveRemoteOffer[] = "have-remote-offer";
static const char kSignalingStateHaveLocalPrAnswer[] = "have-local-pranswer";
static const char kSignalingStateHaveRemotePrAnswer[] = "have-remote-pranswer";
static const char kSignalingStateClosed[] = "closed";

// http://www.w3.org/TR/webrtc/#idl-def-RTCIceGatheringState
static const char kIceGatheringStateNew[] = "new";
static const char kIceGatheringStateGathering[] = "gathering";
static const char kIceGatheringStateComplete[] = "complete";

// http://www.w3.org/TR/webrtc/#idl-def-RTCIceConnectionState
static const char kIceConnectionStateNew[] = "new";
static const char kIceConnectionStatChecking[] = "checking";
static const char kIceConnectionStatConnected[] = "connected";
static const char kIceConnectionStatCompleted[] = "completed";
static const char kIceConnectionStatFailed[] = "failed";
static const char kIceConnectionStatDisconnected[] = "disconnected";
static const char kIceConnectionStatClosed[] = "closed";

// http://www.w3.org/TR/mediacapture-streams/#idl-def-MediaStreamTrackState
static const char kMediaStreamTrackStateNew[] = "new";
static const char kMediaStreamTrackStateLive[] = "live";
static const char kMediaStreamTrackStateEnded[] = "ended";
static const char kMediaStreamTrackStateFailed[] = "failed";

// http://www.w3.org/TR/mediacapture-streams/#idl-def-SourceTypeEnum
static const char kSourceTypeEnumNone[] = "none";
static const char kSourceTypeEnumCamera[] = "camera";
static const char kSourceTypeEnumMicrophone[] = "microphone";

// http://www.w3.org/TR/mediacapture-streams/#idl-def-VideoFacingModeEnum
static const char kVideoFacingModeEnumUser[] = "user";
static const char kVideoFacingModeEnumEnvironment[] = "environment";
static const char kVideoFacingModeEnumLeft[] = "left";
static const char kVideoFacingModeEnumRight[] = "right";

// http://www.w3.org/TR/webrtc/#rtcstats-dictionary
static const char kRTCStatsTypeInboundRtp[] = "inbound-rtp";
static const char kRTCStatsTypeOutboundRtp[] = "outbound-rtp";

// http://www.w3.org/TR/webrtc/#idl-def-RTCDataChannelState
static const char kRTCDataChannelStateConnecting[] = "connecting";
static const char kRTCDataChannelStateOpen[] = "open";
static const char kRTCDataChannelStateClosing[] = "closing";
static const char kRTCDataChannelStateClosed[] = "closed";

// ScreenShare
static const char kDoubangoScreenshareSourceId[] = "X978DoubangoTelecomScreenCapturer785";
static const char kDoubangoScreenshareLabel[] = "ScreenCapturer (Doubango Telecom)";
static const char kDoubangoScreenshareKind[] = "video";

#define kMaxParamArgs 10

#define MAX_ICE_SERVERS 10

typedef struct _ice_server_info {
	char *uri;
	char *username;
	char *password;
} ice_server_info;

WEBRTC_WRAPPER_API int libwebrtc_initialize(); //from _Utils.h in webrtc-everywhere-common
WEBRTC_WRAPPER_API void libwebrtc_deinitialize(); //from _Utils.h in webrtc-everywhere-common

WEBRTC_WRAPPER_API void libeverywhere_take_fake_peer_connection_factory();
WEBRTC_WRAPPER_API void libeverywhere_release_fake_peer_connection_factory();
#ifdef __cplusplus
}
#endif 
#endif // _WEBRTC_WRAPPER_COMMON_H_