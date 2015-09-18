/* Copyright(C) 2014-2015 Doubango Telecom <https://github.com/sarandogou/webrtc-everywhere> */
// http://www.w3.org/TR/webrtc/#idl-def-RTCIceCandidate
#ifndef _WEBRTC_EVERYWHERE_COMMON_RTCICECANDIDATE_H_
#define _WEBRTC_EVERYWHERE_COMMON_RTCICECANDIDATE_H_

#include "config_.hpp"

class WEBRTC_EVERYWHERE_API _RTCIceCandidate
{
public:
	_RTCIceCandidate(const char* candidate = "", const char* sdpMid = "", unsigned short sdpMLineIndex = 0);
	virtual ~_RTCIceCandidate();

	// RTCIceCandidate
	const char* candidate() const { return m_candidate.c_str(); }
	const char* sdpMid() const { return m_sdpMid.c_str(); }
	unsigned short sdpMLineIndex()const { return m_sdpMLineIndex; }

private:
#if _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif
	std::string m_candidate;
	std::string m_sdpMid;
	unsigned short m_sdpMLineIndex;
#if _MSC_VER
#pragma warning(pop)
#endif
};

#endif /* _WEBRTC_EVERYWHERE_COMMON_RTCICECANDIDATE_H_ */
