#include "peer_connection.h"
#include "common_.hpp"
#include "peer_connection_.hpp"
#include "json\json.h"

#include <functional>
#include <map>
#include <string>
#include <memory>
#include <cstring>

using namespace std;

typedef struct peerconnection_ctx_vars_
{
	char* configuration_;
	char* local_description_;
	char* remote_description_;
	signaling_state signaling_state_;
	ice_gathering_state ice_gathering_state_;
	ice_connection_state ice_connection_state_;
} peerconnection_ctx_vars_;

typedef struct peerconnection_ctx_ops_{
	void(*on_negotiation_needed)();
	void(*on_ice_candidate)();
	void(*on_signaling_state_change)();
	void(*on_ice_connection_state_change)();
	void(*on_ice_gathering_state_change)();
} peerconnection_ctx_ops_;

typedef struct rtc_ice_server_ {
	const char* urls;
	const char* username;
	const char* credential;
} rtc_ice_server_;

static map<unsigned int, peerconnection_ctx*> pc_ctx_map_;
 static map<unsigned int, _PeerConnection*> pc_map_;
/*
static void onicecandidateCallback(std::shared_ptr<_RTCPeerConnectionIceEvent> e)
{
	if (e){
		if (e->candidate) {
			peerConnection->AddIceCandidate(e->candidate.get());
		}

		_RTCIceCandidate* p = e->candidate.get();
		const char *candidate = p->candidate();
		printf("\nhaha   %s\n\n", candidate);
	}

}
*/

static void onnegotiationneededCallback()
{
	fprintf(stderr, "onnegotiationneededCallback called\n");
	// call negotiation_needed
}

/*
// http://www.w3.org/TR/webrtc/#event-signalingstatechange
static void onsignalingstatechangeCallback()
{
	if (peerConnection) {
		WE_DEBUG_INFO("New signaling state: %s", peerConnection->SignalingState());
	}
}

static void oniceconnectionstatechangeCallback()
{
	WE_DEBUG_INFO("%s", "oniceconnectionstatechangeCallback");
}

static void onDataChannelCallback(std::shared_ptr < _RTCDataChannelEvent> dc_sets)
{
	WE_DEBUG_INFO("on channel!!!!!!!!!!!!!!!!!!\n");
	_RTCDataChannel *dc = dc_sets.get()->channel.get();
}
*/

WEBRTC_WRAPPER_API peerconnection_ctx* peer_connection_create(const char* configuration, 
	on_negotiation_needed negotiation_needed,
	on_ice_candidate ice_candidate,
	on_signaling_state_change signaling_state_change,
	on_ice_connection_state_change ice_connection_state_change,
	on_ice_gathering_state_change ice_gathering_state_change,
	void* user_data)
{
	static unsigned int peer_connection_count = 0;
	
	//TODO: setup peerconnection_ctx object, and associate user_data inctx
	peerconnection_ctx *ret = (peerconnection_ctx *)calloc(1, sizeof(peerconnection_ctx));
	// setup _PeerConnection object
	_PeerConnection *peerConnection = new _PeerConnection();
	
	if (ret != NULL) {
		unsigned int handle = peer_connection_count++; //FIXME: May have concurrent problem, use mutex
		
		pc_ctx_map_[handle] = ret;
		ret->handle = handle;
		pc_map_[handle] = peerConnection;
	}
	
	//TODO: parse configuration as json

	//TODO: Fake configuration is parsed, and fill in peerConnectionConfiguration:_RTCConfiguration
	static _RTCIceServer iceServers[] = {
		{ "stun:stun.l.google.com:19302" },
		{ "turn:numb.viagenie.ca:3478", "user", "password" },
	};

	static size_t iceServersCount = sizeof(iceServers) / sizeof(iceServers[0]);

	//TODO: Fake configuration is parsed
	_RTCConfiguration peerConnectionConfiguration;
	peerConnectionConfiguration.iceServers.push_back(_RTCIceServer(iceServers[0].uri, iceServers[0].username, iceServers[0].password));
	peerConnectionConfiguration.iceServers.push_back(_RTCIceServer(iceServers[1].uri, iceServers[1].username, iceServers[1].password));


	std::shared_ptr<_MediaConstraints> optional = std::make_shared<_MediaConstraints>();
	std::shared_ptr<_MediaConstraints> mandatory = std::make_shared<_MediaConstraints>();
	_MediaConstraintsObj peerConnectionConstraints(optional, mandatory);

	mandatory->insert(std::pair<std::string, std::string>("offerToReceiveAudio", "false"));
	mandatory->insert(std::pair<std::string, std::string>("offerToReceiveVideo", "false"));
	optional->insert(std::pair<std::string, std::string>("DtlsSrtpKeyAgreement", "true"));
	//optional->insert(std::pair<std::string, std::string>("RtpDataChannels", "true"));



	//peerConnection->SetCallback_onnegotiationneeded(onnegotiationneededCallback);
	//peerConnection->SetCallback_onicecandidate(onicecandidateCallback);
	//peerConnection->SetCallback_onsignalingstatechange(onsignalingstatechangeCallback);
	//peerConnection->SetCallback_oniceconnectionstatechange(oniceconnectionstatechangeCallback);
	//peerConnection->SetCallback_ondatachannel(onDataChannelCallback);

	if (!peerConnection->Init(&peerConnectionConfiguration, &peerConnectionConstraints)) {
		// TODO: do something
	}
	
	printf("SignalingState: %s\n", peerConnection->SignalingState());
	printf("IceGatherState: %s\n", peerConnection->IceGatheringState());
	printf("IceConnectionState: %s\n", peerConnection->IceConnectionState());


	getchar();
	peerConnection->Close();

	//TODO: return peerconnection_ctx object
	return ret;
}

WEBRTC_WRAPPER_API void peer_connection_destroy(peerconnection_ctx* ctx) {
	std::map<unsigned int, peerconnection_ctx*>::iterator it;
	if (ctx != NULL) {
		it = pc_ctx_map_.find(ctx->handle);
		if (it != pc_ctx_map_.end()) {
			pc_ctx_map_.erase(it);
			free(ctx);
		}
	}
}

WEBRTC_WRAPPER_API void* peer_connection_get_user_data(peerconnection_ctx* ctx) {
	return ctx->user_data_;
}

WEBRTC_WRAPPER_API void peer_connection_engage(peerconnection_ctx* ctx) { // easy to meet peers ice negotiation timing

}

WEBRTC_WRAPPER_API void peer_connection_close(peerconnection_ctx* ctx) {

}
