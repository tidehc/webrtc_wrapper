/* Copyright(C) 2014-2015 Doubango Telecom <https://github.com/sarandogou/webrtc-everywhere> */
// http://www.w3.org/TR/mediacapture-streams/#idl-def-MediaStreamConstraints
#include "media_stream_constraints_.hpp"
#include "debug_.hpp"

_MediaStreamConstraints::_MediaStreamConstraints(cpp11::shared_ptr<_MediaTrackConstraints> audio /*= nullPtr*/, cpp11::shared_ptr<_MediaTrackConstraints> video /*= nullPtr*/)
	: m_audio(audio)
	, m_video(video)
{
}

_MediaStreamConstraints::~_MediaStreamConstraints()
{
	m_audio = nullPtr;
	m_video = nullPtr;

	WE_DEBUG_INFO("_MediaStreamConstraints::~_MediaStreamConstraints");
}
