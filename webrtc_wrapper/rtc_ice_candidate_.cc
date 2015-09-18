/* Copyright(C) 2014-2015 Doubango Telecom <https://github.com/sarandogou/webrtc-everywhere> */
// http://www.w3.org/TR/webrtc/#idl-def-RTCIceCandidate
#include "rtc_ice_candidate_.hpp"
#include "debug_.hpp"

_RTCIceCandidate::_RTCIceCandidate(const char* candidate /*= ""*/, const char* sdpMid /*= ""*/, unsigned short sdpMLineIndex /*= 0*/)
	: m_candidate(candidate)
	, m_sdpMid(sdpMid)
	, m_sdpMLineIndex(sdpMLineIndex)
{

}

_RTCIceCandidate::~_RTCIceCandidate()
{
	WE_DEBUG_INFO("_RTCIceCandidate::~_RTCIceCandidate");
}
