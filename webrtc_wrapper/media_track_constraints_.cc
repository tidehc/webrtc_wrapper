/* Copyright(C) 2014-2015 Doubango Telecom <https://github.com/sarandogou/webrtc-everywhere> */
#include "media_track_constraints_.hpp"
#include "debug_.hpp"

_MediaTrackConstraints::_MediaTrackConstraints(bool bVal)
: m_bValue(bVal)
, m_eType(_MediaTrackConstraintsTypeBoolean)
{
}

_MediaTrackConstraints::_MediaTrackConstraints(cpp11::shared_ptr<_MediaConstraints> mandatory /*= nullPtr*/, cpp11::shared_ptr<_MediaConstraints> optional /*= nullPtr*/)
	: m_mandatory(mandatory)
	, m_optional(optional)
	, m_eType(_MediaTrackConstraintsTypeComposite)
{
}

_MediaTrackConstraints::~_MediaTrackConstraints()
{
	WE_DEBUG_INFO("_MediaTrackConstraints::~_MediaTrackConstraints");
}

