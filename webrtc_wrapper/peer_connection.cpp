#include "peer_connection.h"
#include "common_.hpp"
#include "peer_connection_.hpp"
#include "session_description_.hpp"
#include "buffer_.hpp"

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
	on_negotiation_needed_callback on_negotiation_needed_cb;
	on_ice_candidate_callback on_ice_candidate_cb;
	on_signaling_state_change_callback on_signaling_state_change_cb;
	on_ice_connection_state_change_callback on_ice_connection_state_change_cb;
	on_ice_gathering_state_change_callback on_ice_gathering_state_change_cb;

	on_rtc_session_description_callback create_offer_success_cb;
	on_rtc_peer_connection_error create_offer_failure_cb;
} peerconnection_ctx_ops_;

typedef struct rtc_ice_server_ {
	const char* urls;
	const char* username;
	const char* credential;
} rtc_ice_server_;

static map<unsigned int, peerconnection_ctx*> pc_ctx_map_;
static map<unsigned int, _PeerConnection*> pc_map_;


// http://www.w3.org/TR/webrtc/#event-negotiation
static void onnegotiationneededCallback(unsigned int handle)
{
	fprintf(stderr, "*INFO: onnegotiationneededCallback called\n");
	peerconnection_ctx *ctx = pc_ctx_map_[handle];
}


// http://www.w3.org/TR/webrtc/#event-icecandidate
static void onicecandidateCallback(std::shared_ptr<_RTCPeerConnectionIceEvent> e, unsigned int handle)
{
	_PeerConnection *peerConnection = pc_map_[handle];
	if (e){
		if (e->candidate) {
			peerConnection->AddIceCandidate(e->candidate.get());
		}

		peerconnection_ctx *ctx = pc_ctx_map_[handle];
		
		/*
		_RTCIceCandidate* p = e->candidate.get();
		const char *candidate = p->candidate();
		printf("\nhaha   %s\n\n", candidate);
		*/
	}

}


/*
// http://www.w3.org/TR/webrtc/#event-iceconnectionstatechange
static void oniceconnectionstatechangeCallback()
{
	fprintf(stderr, "*INFO: oniceconnectionstatechangeCallback called");
}

// http://www.w3.org/TR/webrtc/#event-signalingstatechange
static void onsignalingstatechangeCallback()
{
	if (peerConnection) {
		fprintf(stderr, "*INFO: New signaling state: %s", peerConnection->SignalingState());
	}
}

static void icecandidateErrorCb(std::shared_ptr<std::string> error)
{
	fprintf(stderr, "*INFO: candidate fail, %s\n", error.get());
}



*/

static void CreateOfferSuccessCb(std::shared_ptr<_SessionDescription> sdp, unsigned int handle)
{
	// copy the sdp from _PeerConnction obj to peerconnection_ctx obj
	peerconnection_ctx *ctx = pc_ctx_map_[handle];
	char *ctx_local_description = ctx->vars->local_description_;
	if (ctx_local_description == NULL)
		ctx_local_description = (char *)calloc(1, 2048);
	
	_SessionDescription *sdp_ = sdp.get();
	const char *sdp_str = (const char *)sdp->getSdp()->getPtr();
	const char *type_str = (const char *)sdp->getType()->getPtr();

	strncpy(ctx_local_description, sdp_str, strlen(sdp_str));

	//call the callback of peerconnection_ctx
	rtc_session_description *rtc_sdp = (rtc_session_description *)calloc(1, sizeof(rtc_session_description));
	rtc_sdp->type = strdup(type_str);
	rtc_sdp->sdp = strdup(sdp_str);
	ctx->ops->create_offer_success_cb(ctx, rtc_sdp);
}


// http://www.w3.org/TR/webrtc/#idl-def-RTCPeerConnectionErrorCallback
static void CreateOfferErrorCb(std::shared_ptr<std::string> error)
{

}

WEBRTC_WRAPPER_API peerconnection_ctx* peer_connection_create(const char* configuration, 
	on_negotiation_needed_callback negotiation_needed,
	on_ice_candidate_callback ice_candidate,
	on_signaling_state_change_callback signaling_state_change,
	on_ice_connection_state_change_callback ice_connection_state_change,
	on_ice_gathering_state_change_callback ice_gathering_state_change,
	void* user_data)
{
	static unsigned int peer_connection_count = 0;
	
	// construct peerconnection_ctx structure and link the callback functions
	peerconnection_ctx *ret = (peerconnection_ctx *)calloc(1, sizeof(peerconnection_ctx));
	ret->ops = (peerconnection_ctx_ops_ *)calloc(1, sizeof(peerconnection_ctx_ops_));
	ret->vars = (peerconnection_ctx_vars_ *)calloc(1, sizeof(peerconnection_ctx_vars_));
	peerconnection_ctx_ops_ *ret_ops = ret->ops;
	ret_ops->on_negotiation_needed_cb = negotiation_needed;
	ret_ops->on_ice_candidate_cb = ice_candidate;
	ret_ops->on_signaling_state_change_cb = signaling_state_change;
	ret_ops->on_ice_connection_state_change_cb = ice_connection_state_change;
	ret_ops->on_ice_gathering_state_change_cb = ice_gathering_state_change;
	
	Json::Reader reader;
	Json::Value root;
	std::string ctx_config(configuration);
	_RTCConfiguration peerConnectionConfiguration;
	if (reader.parse(ctx_config, root, false))
	{
		for (unsigned int i = 0; i < root.size(); i++)
		{
			_RTCIceServer iceServer_(root[i]["urls"].asString());
			peerConnectionConfiguration.iceServers.push_back(iceServer_);
		}
	}
	// setup _PeerConnection object
	_PeerConnection *peerConnection = new _PeerConnection();

	/* not sure what the use of iceServerCount */
	//static size_t iceServersCount = sizeof(iceServers) / sizeof(iceServers[0]);

	std::shared_ptr<_MediaConstraints> optional = std::make_shared<_MediaConstraints>();
	std::shared_ptr<_MediaConstraints> mandatory = std::make_shared<_MediaConstraints>();
	_MediaConstraintsObj peerConnectionConstraints(optional, mandatory);

	mandatory->insert(std::pair<std::string, std::string>("offerToReceiveAudio", "false"));
	mandatory->insert(std::pair<std::string, std::string>("offerToReceiveVideo", "false"));
	optional->insert(std::pair<std::string, std::string>("DtlsSrtpKeyAgreement", "true"));
	//optional->insert(std::pair<std::string, std::string>("RtpDataChannels", "true"));

	peerConnection->SetCallback_onnegotiationneeded(onnegotiationneededCallback);
	peerConnection->SetCallback_onicecandidate(onicecandidateCallback);
	//peerConnection->SetCallback_onsignalingstatechange(onsignalingstatechangeCallback);
	//peerConnection->SetCallback_oniceconnectionstatechange(oniceconnectionstatechangeCallback);
	//peerConnection->SetCallback_ondatachannel(onDataChannelCallback);

	if (!peerConnection->Init(&peerConnectionConfiguration, &peerConnectionConstraints)) {
		// TODO: do something
	}


	if (ret != NULL && peerConnection->IsInitialized()) {
		unsigned int handle = peer_connection_count++; //FIXME: May have concurrent problem, use mutex
		pc_ctx_map_[handle] = ret;
		ret->handle = handle;
		pc_map_[handle] = peerConnection;
		peerConnection->setHandle(handle);
	}

	//std::shared_ptr<_RTCDataChannelInit> datachannelConfig = std::make_shared<_RTCDataChannelInit>();
	//peerConnection->CreateDataChannel("demo", datachannelConfig);

	//peerConnection->Close();

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

WEBRTC_WRAPPER_API int peer_connection_create_offer(peerconnection_ctx* ctx, on_rtc_session_description_callback success, on_rtc_peer_connection_error failure)
{
	unsigned int handle = ctx->handle;
	_PeerConnection *peerConnection = pc_map_[handle];
	peerconnection_ctx_ops_ *ops = ctx->ops;
	ops->create_offer_success_cb = success;
	ops->create_offer_failure_cb = failure;
	peerConnection->CreateOffer(CreateOfferSuccessCb, CreateOfferErrorCb);
	return 1; //uniray: I'm not sure?
}

WEBRTC_WRAPPER_API void* peer_connection_get_user_data(peerconnection_ctx* ctx) {
	return ctx->user_data_;
}

WEBRTC_WRAPPER_API void peer_connection_engage(peerconnection_ctx* ctx) { // easy to meet peers ice negotiation timing

}

WEBRTC_WRAPPER_API void peer_connection_close(peerconnection_ctx* ctx) {

}
