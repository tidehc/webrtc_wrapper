#include "peer_connection.h"
#include "rtc_data_channel.h"

#include "common_.hpp"
#include "peer_connection_.hpp"
#include "session_description_.hpp"
#include "buffer_.hpp"
#include "rtc_data_channel_.hpp"
#include "rtc_ice_candidate_.hpp"

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
	_rtc_session_description* local_description_;
	_rtc_session_description* remote_description_;
	signaling_state signaling_state_;
	ice_gathering_state ice_gathering_state_;
	ice_connection_state ice_connection_state_;
} peerconnection_ctx_vars_;

typedef struct peerconnection_ctx_ops_
{
	on_negotiation_needed_callback on_negotiation_needed_cb;
	on_ice_candidate_callback on_ice_candidate_cb;
	on_signaling_state_change_callback on_signaling_state_change_cb;
	on_ice_connection_state_change_callback on_ice_connection_state_change_cb;
	on_ice_gathering_state_change_callback on_ice_gathering_state_change_cb;

	on_rtc_session_description_callback create_offer_success_cb;
	on_rtc_peer_connection_error create_offer_failure_cb;
	on_void_function set_localSDP_success_cb;
	on_rtc_peer_connection_error set_localSDP_failure_cb;
} peerconnection_ctx_ops_;

typedef struct rtc_ice_server_ {
	const char* urls;
	const char* username;
	const char* credential;
} rtc_ice_server_;

static map<unsigned int, pair<peerconnection_ctx*, _PeerConnection*> > pc_map_;
static map<unsigned int, pair<datachannel_ctx*, _RTCDataChannel*> > dc_map_;


// peer_connection callbacks
// http://www.w3.org/TR/webrtc/#event-negotiation
static void onnegotiationneededCallback(unsigned int handle)
{
	fprintf(stderr, "*INFO: onnegotiationneededCallback called\n");
	peerconnection_ctx *ctx = pc_map_[handle].first;
	ctx->ops->on_negotiation_needed_cb(ctx);
}

// http://www.w3.org/TR/webrtc/#event-icecandidate
static void onicecandidateCallback(std::shared_ptr<_RTCPeerConnectionIceEvent> e, unsigned int handle)
{
	fprintf(stderr, "*INFO: onicecandidateCallback called\n");
	_PeerConnection *peerConnection = pc_map_[handle].second;
	if (e){
		if (e->candidate) {
			//peerConnection->AddIceCandidate(e->candidate.get());
			peerconnection_ctx *ctx = pc_map_[handle].first;

			_RTCIceCandidate* p = e->candidate.get();
			const char *candidate = p->candidate();
			ctx->ops->on_ice_candidate_cb(ctx, candidate);
		}	
	}
}

// http://www.w3.org/TR/webrtc/#event-iceconnectionstatechange
static void oniceconnectionstatechangeCallback(unsigned int handle)
{
	fprintf(stderr, "*INFO: oniceconnectionstatechangeCallback called\n");
	_PeerConnection *peerConnection = pc_map_[handle].second;
	peerconnection_ctx *ctx = pc_map_[handle].first;
	//overwrite the ice connection state of ctx and call the ctx's callback
	if (strcmp(peerConnection->SignalingState(), "new"))
		ctx->vars->ice_connection_state_ = CONN_STATE_NEW;
	else if (strcmp(peerConnection->SignalingState(), "checking"))
		ctx->vars->ice_connection_state_ = CONN_STATE_CHECKING;
	else if (strcmp(peerConnection->SignalingState(), "connected"))
		ctx->vars->ice_connection_state_ = CONN_STATE_CONNECTED;
	else if (strcmp(peerConnection->SignalingState(), "completed"))
		ctx->vars->ice_connection_state_ = CONN_STATE_COMPLETED;
	else if (strcmp(peerConnection->SignalingState(), "failed"))
		ctx->vars->ice_connection_state_ = CONN_STATE_FAILED;
	else if (strcmp(peerConnection->SignalingState(), "disconnected"))
		ctx->vars->ice_connection_state_ = CONN_STATE_DISCONNECTED;
	else if (strcmp(peerConnection->SignalingState(), "closed"))
		ctx->vars->ice_connection_state_ = CONN_STATE_CLOSED;

	ctx->ops->on_ice_connection_state_change_cb(ctx, ctx->vars->ice_connection_state_);
}

// http://www.w3.org/TR/webrtc/#event-signalingstatechange
static void onsignalingstatechangeCallback(unsigned int handle)
{
	fprintf(stderr, "*INFO: onsignalingstatechangeCallback called\n");
	_PeerConnection *peerConnection = pc_map_[handle].second;
	peerconnection_ctx *ctx = pc_map_[handle].first;
	
	//overwrite the signaling state of ctx and call the ctx's callback
	if (strcmp(peerConnection->SignalingState(), "stable"))
		ctx->vars->signaling_state_ = SIG_STATE_STABLE;
	else if (strcmp(peerConnection->SignalingState(), "have-local-offer"))
		ctx->vars->signaling_state_ = SIG_STATE_HAVE_LOCAL_OFFER;
	else if (strcmp(peerConnection->SignalingState(), "have-remote-offer"))
		ctx->vars->signaling_state_ = SIG_STATE_HAVE_REMOTE_OFFER;
	else if (strcmp(peerConnection->SignalingState(), "have-local-pranswer"))
		ctx->vars->signaling_state_ = SIG_STATE_HAVE_LOCAL_PRANSWER;
	else if (strcmp(peerConnection->SignalingState(), "have-remote-pranswer"))
		ctx->vars->signaling_state_ = SIG_STATE_HAVE_REMOTE_PRANSWER;
	else if (strcmp(peerConnection->SignalingState(), "closed"))
		ctx->vars->signaling_state_ = SIG_STATE_CLOSED;

	ctx->ops->on_signaling_state_change_cb(ctx, ctx->vars->signaling_state_);
}

static void icecandidateErrorCb(std::shared_ptr<std::string> error)
{
	fprintf(stderr, "*INFO: candidate fail, %s\n", error.get());
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
	ret->vars->local_description_ = (rtc_session_description *)calloc(1, sizeof(rtc_session_description));
	ret->vars->remote_description_ = (rtc_session_description *)calloc(1, sizeof(rtc_session_description));

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
	peerConnection->SetCallback_onsignalingstatechange(onsignalingstatechangeCallback);
	peerConnection->SetCallback_oniceconnectionstatechange(oniceconnectionstatechangeCallback);
	//peerConnection->SetCallback_ondatachannel(onDataChannelCallback);

	if (!peerConnection->Init(&peerConnectionConfiguration, &peerConnectionConstraints)) {
		// TODO: do something
	}


	if (ret != NULL && peerConnection->IsInitialized()) {
		unsigned int handle = peer_connection_count++; //FIXME: May have concurrent problem, use mutex
		pc_map_[handle] = make_pair(ret, peerConnection);
		ret->handle = handle;
		peerConnection->setHandle(handle);
	}

	return ret;
}


static void CreateOfferSuccessCb(std::shared_ptr<_SessionDescription> sdp, unsigned int handle)
{
	// copy the sdp from _PeerConnction obj to peerconnection_ctx obj
	peerconnection_ctx *ctx = pc_map_[handle].first;
	/* TODO: need to update ctx->vars->local_description_ ? 
		I don't think so because ctx->vars->local_description_ is encapsulated.
		wrapper users won't care it is updated.
	*/
	char *type_str = (char *)sdp.get()->getType()->getPtr();
	char *sdp_str = (char *)sdp.get()->getSdp()->getPtr();
	size_t type_size = sdp.get()->getType()->getSize();
	size_t sdp_size = sdp.get()->getSdp()->getSize();

	rtc_session_description *rtc_sdp = (rtc_session_description *)calloc(1, sizeof(rtc_session_description));
	rtc_sdp->type = (char *)calloc(1, type_size+1);
	rtc_sdp->sdp = (char *)calloc(1, sdp_size+1);
	strncpy(rtc_sdp->type, type_str, type_size);
	strncpy(rtc_sdp->sdp, sdp_str, sdp_size);
	//call the callback of peerconnection_ctx
	ctx->ops->create_offer_success_cb(ctx, rtc_sdp);
}

// http://www.w3.org/TR/webrtc/#idl-def-RTCPeerConnectionErrorCallback
static void CreateOfferErrorCb(std::shared_ptr<std::string> error)
{

}

WEBRTC_WRAPPER_API int peer_connection_create_offer(peerconnection_ctx* ctx, 
		on_rtc_session_description_callback success, 
		on_rtc_peer_connection_error failure)
{
	unsigned int handle = ctx->handle;
	_PeerConnection *peerConnection = pc_map_[handle].second;
	peerconnection_ctx_ops_ *ops = ctx->ops;
	ops->create_offer_success_cb = success;
	ops->create_offer_failure_cb = failure;
	peerConnection->CreateOffer(CreateOfferSuccessCb, CreateOfferErrorCb);
	return 1; //uniray: I'm not sure?
}

WEBRTC_WRAPPER_API int peer_connection_create_answer(peerconnection_ctx* ctx,
	on_rtc_session_description_callback success,
	on_rtc_peer_connection_error failure)
{
	return 1;
}


WEBRTC_WRAPPER_API void peer_connection_set_local_description(peerconnection_ctx* ctx,
	rtc_session_description* description,
	on_void_function success,
	on_rtc_peer_connection_error failure)
{
	unsigned int handle = ctx->handle;
	_PeerConnection *peerConnection = pc_map_[handle].second;
	peerconnection_ctx_ops_ *ops = ctx->ops;
	ops->set_localSDP_success_cb = success;
	ops->set_localSDP_failure_cb = failure;

	const char *sdpPtr = description->sdp;
	size_t sdpSize = strlen(sdpPtr);
	const char *typePtr = description->type;
	size_t typeSize = strlen(typePtr);

	_SessionDescription *sdp = new _SessionDescription(sdpPtr, sdpSize, typePtr, typeSize);

	peerConnection->SetLocalDescription(sdp, success, failure);
}

WEBRTC_WRAPPER_API rtc_session_description* peer_connection_local_description(peerconnection_ctx* ctx){
	_PeerConnection *peerConnection = pc_map_[ctx->handle].second;
	
	char *sdp = ctx->vars->local_description_->sdp;
	char *type = ctx->vars->local_description_->type;
	size_t sdp_size = peerConnection->LocalDescription()->getSdp()->getSize();
	size_t type_size = peerConnection->LocalDescription()->getType()->getSize();

	if (sdp != NULL)
	{
		free(sdp);
		sdp = NULL;
	}

	if (type != NULL)
	{
		free(type);
		type = NULL;
	}

	sdp = (char *)calloc(1, sdp_size+1);
	type = (char *)calloc(1, type_size+1);
	
	strncpy(sdp, (const char*)peerConnection->LocalDescription()->getSdp()->getPtr(), sdp_size);
	strncpy(type, (const char *)peerConnection->LocalDescription()->getType()->getPtr(), type_size);
	ctx->vars->local_description_->sdp = sdp;
	ctx->vars->local_description_->type = type;
	return ctx->vars->local_description_;
}


WEBRTC_WRAPPER_API void peer_connection_set_remote_description(peerconnection_ctx* ctx, rtc_session_description* description, on_void_function success, on_rtc_peer_connection_error failure)
{
	unsigned int handle = ctx->handle;
	_PeerConnection *peerConnection = pc_map_[handle].second;
	peerconnection_ctx_ops_ *ops = ctx->ops;
	ops->set_localSDP_success_cb = success;
	ops->set_localSDP_failure_cb = failure;

	const char *sdpPtr = description->sdp;
	size_t sdpSize = strlen(sdpPtr);
	const char *typePtr = description->type;
	size_t typeSize = strlen(typePtr);

	_SessionDescription *sdp = new _SessionDescription(sdpPtr, sdpSize, typePtr, typeSize);

	peerConnection->SetRemoteDescription(sdp, success, failure);
}

WEBRTC_WRAPPER_API rtc_session_description* peer_connection_remote_description(peerconnection_ctx* ctx);

WEBRTC_WRAPPER_API signaling_state peer_connection_signaling_state(peerconnection_ctx* ctx);

WEBRTC_WRAPPER_API void peer_connection_add_ice_candidate(peerconnection_ctx* ctx,
	const char* candidate,
	on_void_function success,
	on_rtc_peer_connection_error failure)
{
	/* TODO: the success & failure callback are not implemented */
	_PeerConnection *peerConnection = pc_map_[ctx->handle].second;

	/* TODO: not sure the initialization is right */
	_RTCIceCandidate *candidate_ = new _RTCIceCandidate(candidate, "data", 0);
	peerConnection->AddIceCandidate(candidate_);
}

WEBRTC_WRAPPER_API ice_gathering_state peer_connection_ice_gathering_state(peerconnection_ctx* ctx);

WEBRTC_WRAPPER_API ice_connection_state peer_connection_ice_connection_state(peerconnection_ctx* ctx);

WEBRTC_WRAPPER_API void peer_connection_close(peerconnection_ctx* ctx) {

}


WEBRTC_WRAPPER_API void peer_connection_destroy(peerconnection_ctx* ctx) {
	std::map<unsigned int, pair<peerconnection_ctx*, _PeerConnection*> >::iterator it;
	if (ctx != NULL) {
		it = pc_map_.find(ctx->handle);
		if (it != pc_map_.end()) {
			pc_map_.erase(it);
			free(ctx);
		}
	}
}
/* TODO: will be implemented later



WEBRTC_WRAPPER_API void* peer_connection_get_user_data(peerconnection_ctx* ctx) {
	return ctx->user_data_;
}

WEBRTC_WRAPPER_API void peer_connection_engage(peerconnection_ctx* ctx) { // easy to meet peers ice negotiation timing

}
*/


// datachannel callback
static void onOpenCallback()
{
	fprintf(stderr, "*INFO: on open!!!!!!!!!!!!!!!!!!\n");
}

WEBRTC_WRAPPER_API datachannel_ctx* peer_connection_create_datachannel(peerconnection_ctx* ctx,
		const char* label,  /*TODO: datachannel config*/
		on_datachannel_open_callback on_open_cb,
		on_datachannel_message_callback on_message_cb,
		on_datachannel_error_callback on_error_cb,
		on_datachannel_close_callback on_close_cb)
{
	static unsigned int datachannel_count = 0;
	// initialize a datachannel_ctx obj
	datachannel_ctx *ret = create_datachannel_ctx();
	
	/* TODO: uniray: I don't know what's used for? */
	set_datachannel_cb_onopen(ret, on_open_cb);
	set_datachannel_cb_onmessage(ret, on_message_cb);
	set_datachannel_cb_onerror(ret, on_error_cb);
	set_datachannel_cb_onclose(ret, on_close_cb);


	// _PeerConnection create datachannel
	_PeerConnection *peerConnection = pc_map_[ctx->handle].second;
	std::shared_ptr<_RTCDataChannelInit> datachannelConfig = std::make_shared<_RTCDataChannelInit>();
	cpp11::shared_ptr<_RTCDataChannel> dataChannel = nullptr;
	if (!(dataChannel = peerConnection->CreateDataChannel(label, datachannelConfig))) {
		// TODO: do something
	}

	dataChannel->onopenSet(onOpenCallback);	
	//dataChannel->onerrorSet()
	
	if (ret != NULL) {
		unsigned int handle = datachannel_count++; //FIXME: May have concurrent problem, use mutex
		dc_map_[handle] = make_pair(ret, dataChannel.get());
		ret->handle = handle;
	}
	return NULL;
}


WEBRTC_WRAPPER_API rtc_session_description *peer_connection_initial_SDP(const char *type,
	const char *sdp)
{
	rtc_session_description *session_description = (rtc_session_description *)calloc(1, sizeof(rtc_session_description));
	session_description->type = strdup(type);
	session_description->sdp = strdup(sdp);
	return session_description;
}