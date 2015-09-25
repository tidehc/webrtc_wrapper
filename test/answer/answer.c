#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "dummy.h"
#include "peer_connection.h"
#include "rtc_data_channel.h"

void _on_negotiation_needed(peerconnection_ctx* ctx)
{
	fprintf(stderr, "\nwebrtc_wrapper: _on_negotiation_needed called\n\n");
}

void _on_ice_candidate(peerconnection_ctx* ctx, const char *candidate)
{
	fprintf(stderr, "\nwebrtc_wrapper: _on_ice_candidate\n");
	fprintf(stderr, "got candidate: %s\n\n", candidate);

	if (candidate)
		peer_connection_add_ice_candidate(ctx, candidate, NULL, NULL);/* TODO: the success & failure callback are not implemented */
}

void _on_signaling_state_change(peerconnection_ctx* ctx, signaling_state state)
{
	fprintf(stderr, "\nwebrtc_wrapper: _on_signaling_state_change: %d\n\n", state);
}

void _on_ice_connection_state_change(peerconnection_ctx* ctx, ice_connection_state state)
{
	fprintf(stderr, "\nwebrtc_wrapper: _on_ice_connection_state_change: %d\n\n", state);
}

void _on_ice_gathering_state_change(peerconnection_ctx* ctx, ice_gathering_state state)
{
	fprintf(stderr, "\nwebrtc_wrapper: _on_ice_gathering_state_change\n\n");
}



void _on_set_localSDP_success()
{
	fprintf(stderr, "_on_create_offer_success????????????????????????????\n");
}

void _on_set_localSDP_failure(const char *error)
{
	fprintf(stderr, "_on_set_localSDP_failure:%s\n", error);
}


void _on_set_remoteSDP_success(/*TODO: need add ctx??*/)
{
	fprintf(stderr, "\n\n_on_set_remoteSDP_success   ????????????????????????????\n\n");
}

void _on_set_remoteSDP_failure(const char *error)
{
	fprintf(stderr, "\n\n _on_set_localSDP_failure:%s !!!!!!\n\n", error);
}

// datachannel callback
void on_open_()
{

}

void on_close_()
{}

void on_message_()
{}

void on_error_(const char *error)
{}

int main(int argc, char* argv[])
{
	libwebrtc_initialize();
	libeverywhere_take_fake_peer_connection_factory();

	const char* configuration_json = "[ { \"urls\": \"stun:stun.l.google.com:19302\" }, { \"urls\": \"turn : turn.example.org\", \"username\": \"user\", \"credential\": \"myPassword\" } ] ";

	peerconnection_ctx* ctx = peer_connection_create(configuration_json,
		&_on_negotiation_needed,
		&_on_ice_candidate,
		&_on_signaling_state_change,
		&_on_ice_connection_state_change,
		&_on_ice_gathering_state_change,
		NULL);

	peer_connection_create_datachannel(ctx, "demo",
		&on_open_,
		&on_message_,
		&on_error_,
		&on_close_);


	FILE *pFile;
	pFile = fopen("..\\SDP\\offerSDP", "rb");
	char offerSDP_str[2048];
	memset(offerSDP_str, 0, 2048);
	fread(offerSDP_str, 1, 2048, pFile);
	fclose(pFile);
	printf("offerSDP: \n%s\n", offerSDP_str);

	rtc_session_description *remote_description = peer_connection_initial_SDP(offerSDP_str, strlen(offerSDP_str), "offer", 5);
	peer_connection_set_remote_description(ctx, remote_description, _on_set_remoteSDP_success, _on_set_remoteSDP_failure);


		
	getchar();

	peer_connection_destroy(ctx);

	// Library resources should be released after all other resources be released
	libwebrtc_deinitialize();
	//libeverywhere_release_fake_peer_connection_factory();

	return 0;
}