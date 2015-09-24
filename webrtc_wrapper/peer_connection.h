/* Copyright(C) 2015 Toppano <https://github.com/sukinull/libwebrtc> */
#ifndef _WEBRTC_WRAPPER_PEER_CONNECTION_H_
#define _WEBRTC_WRAPPER_PEER_CONNECTION_H_
#include "common.h"
#include "rtc_data_channel.h"

#ifdef __cplusplus
extern "C" {
#endif

	struct peerconnection_ctx_vars_;
	struct peerconnection_ctx_ops_;
	
	//http://www.w3.org/TR/2015/WD-webrtc-20150210/#rtcpeerstate-enum
	typedef enum _signaling_state {
		SIG_STATE_STABLE,
		SIG_STATE_HAVE_LOCAL_OFFER,
		SIG_STATE_HAVE_REMOTE_OFFER,
		SIG_STATE_HAVE_LOCAL_PRANSWER,
		SIG_STATE_HAVE_REMOTE_PRANSWER,
		SIG_STATE_CLOSED,
	} signaling_state;

	//http://www.w3.org/TR/2015/WD-webrtc-20150210/#rtcicegatheringstate-enum
	typedef enum _ice_gathering_state {
		GATHER_STATE_NEW,
		GATHER_STATE_GATHERING,  
		GATHER_STATE_COMPLETE
	} ice_gathering_state;

	//http://www.w3.org/TR/2015/WD-webrtc-20150210/#rtciceconnectionstate-enum
	typedef enum _ice_connection_state {
		CONN_STATE_NEW,
		CONN_STATE_CHECKING,
		CONN_STATE_CONNECTED,
		CONN_STATE_COMPLETED,
		CONN_STATE_FAILED,
		CONN_STATE_DISCONNECTED,
		CONN_STATE_CLOSED
	} ice_connection_state;

	typedef struct _rtc_session_description {
		char* type;
		char* sdp;
	} rtc_session_description;

	typedef struct peerconnection_ctx_ {
		unsigned int handle;
		struct peerconnection_ctx_vars_ * vars;
		struct peerconnection_ctx_ops_ * ops;
		void* user_data_;
	} peerconnection_ctx;

	typedef void (*on_negotiation_needed_callback)(peerconnection_ctx*);
	typedef void (*on_ice_candidate_callback)(peerconnection_ctx* , const char* /*TODO: const char* canidate*/);
	typedef void (*on_signaling_state_change_callback)(peerconnection_ctx*, signaling_state /*signaling_state*/);
	typedef void (*on_ice_connection_state_change_callback)(peerconnection_ctx*, ice_connection_state /*ice_connection_state*/);
	typedef void (*on_ice_gathering_state_change_callback)(peerconnection_ctx*, ice_gathering_state /*ice_gathering_state*/);

	// http://www.w3.org/TR/webrtc/#legacy-interface-extensions
	
	// http://www.w3.org/TR/webrtc/#idl-def-RTCSessionDescriptionCallback
	typedef void(*on_rtc_session_description_callback)(peerconnection_ctx*, rtc_session_description*);
	
	typedef void(*on_void_function)();
	
	//http://www.w3.org/TR/webrtc/#idl-def-RTCPeerConnectionErrorCallback
	typedef void(*on_rtc_peer_connection_error)(const char *error);
	
	// https://w3c.github.io/webrtc-pc/#idl-def-RTCPeerConnection
	/**
	 * @param configuration TestCase 1 - { [ { "urls": "stun.l.google.com:19302" }, { "urls": "turn:turn.example.org", "username": "user", "credential": "myPassword" } ] }
	 */


	WEBRTC_WRAPPER_API peerconnection_ctx* peer_connection_create(const char* configuration, //
				on_negotiation_needed_callback negotiation_needed,
				on_ice_candidate_callback ice_candidate,
				on_signaling_state_change_callback signaling_state_change,
				on_ice_connection_state_change_callback ice_connection_state_change,
				on_ice_gathering_state_change_callback ice_gathering_state_change,
				void* user_data);

	// http://www.w3.org/TR/webrtc/#legacy-interface-extensions
	WEBRTC_WRAPPER_API int peer_connection_create_offer(peerconnection_ctx* ctx, 
				on_rtc_session_description_callback success, 
				on_rtc_peer_connection_error failure);

	WEBRTC_WRAPPER_API int peer_connection_create_answer(peerconnection_ctx* ctx, 
				on_rtc_session_description_callback success, 
				on_rtc_peer_connection_error failure);

	/**
	* @param description TestCase 1:
	*/
	WEBRTC_WRAPPER_API void peer_connection_set_local_description(peerconnection_ctx* ctx, 
				rtc_session_description* description, 
				on_void_function success, 
				on_rtc_peer_connection_error failure);

	WEBRTC_WRAPPER_API rtc_session_description* peer_connection_local_description(peerconnection_ctx* ctx);

	WEBRTC_WRAPPER_API void peer_connection_set_remote_description(peerconnection_ctx* ctx, 
				rtc_session_description* description, 
				on_void_function success, 
				on_rtc_peer_connection_error failure);

	WEBRTC_WRAPPER_API rtc_session_description* peer_connection_remote_description(peerconnection_ctx* ctx);

	WEBRTC_WRAPPER_API signaling_state peer_connection_signaling_state(peerconnection_ctx* ctx);
	
	WEBRTC_WRAPPER_API void peer_connection_add_ice_candidate(peerconnection_ctx* ctx,
				const char* candidate, 
				on_void_function success, 
				on_rtc_peer_connection_error failure);

	WEBRTC_WRAPPER_API ice_gathering_state peer_connection_ice_gathering_state(peerconnection_ctx* ctx);
	
	WEBRTC_WRAPPER_API ice_connection_state peer_connection_ice_connection_state(peerconnection_ctx* ctx);


	WEBRTC_WRAPPER_API void peer_connection_close(peerconnection_ctx* ctx);
	
	WEBRTC_WRAPPER_API void peer_connection_destroy(peerconnection_ctx* ctx);
	//int peer_connection_can_trickle_ice_candidates();
	//
	//WEBRTC_WRAPPER_API void* peer_connection_get_user_data(peerconnection_ctx* ctx);
	//WEBRTC_WRAPPER_API void peer_connection_engage(peerconnection_ctx* ctx); // easy to meet peers ice negotiation timing

	// http://www.w3.org/TR/2015/WD-webrtc-20150210/#rtcpeerconnection-interface-extensions
	WEBRTC_WRAPPER_API datachannel_ctx* peer_connection_create_datachannel(peerconnection_ctx* ctx,
				const char* label,  /*TODO: datachannel config*/
				on_datachannel_open_callback on_open_cb,
				on_datachannel_message_callback on_message_cb,
				on_datachannel_error_callback on_error_cb,
				on_datachannel_close_callback on_close_cb);

#ifdef __cplusplus
}
#endif
#endif // _WEBRTC_WRAPPER_PEER_CONNECTION_H_
