#ifndef _WEBRTC_WRAPPER_DUMMY_H_
#define _WEBRTC_WRAPPER_DUMMY_H_

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

// http://www.html5rocks.com/en/tutorials/webrtc/basics/
struct MediaStream;
struct RTCPeerConnection;
struct RTCDataChannel;

WEBRTC_WRAPPER_API void hi();

#ifdef __cplusplus
}
#endif
#endif // _WEBRTC_WRAPPER_DUMMY_H_