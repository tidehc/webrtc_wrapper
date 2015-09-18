#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "dummy.h"
#include "peer_connection.h"

void _on_negotiation_needed(peerconnection_ctx* ctx)
{

}


void _on_ice_candidate(peerconnection_ctx* ctx/*TODO: const char* canidate*/)
{

}

void _on_signaling_state_change(peerconnection_ctx* ctx, int state)
{

}

void _on_ice_connection_state_change(peerconnection_ctx* ctx, int state)
{

}

void _on_ice_gathering_state_change(peerconnection_ctx* ctx, ice_gathering_state state)
{

}

int main(int argc, char* argv[])
{
	//hi();

	libwebrtc_initialize();
	libeverywhere_take_fake_peer_connection_factory();

	const char* configuration_json = "{ [ { \"urls\": \"stun.l.google.com:19302\" }, { \"urls\": \"turn : turn.example.org\", \"username\": \"user\", \"credential\": \"myPassword\" } ] }";
	peerconnection_ctx* ctx = peer_connection_create(configuration_json,
		&_on_negotiation_needed,
		&_on_ice_candidate,
		&_on_signaling_state_change,
		&_on_ice_connection_state_change,
		&_on_ice_gathering_state_change,
		NULL);


	peer_connection_destroy(ctx);



	// Library resources should be released after all other resources be released
	libwebrtc_deinitialize();
	//libeverywhere_release_fake_peer_connection_factory();
	return 0;
}