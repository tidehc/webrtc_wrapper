/* Copyright(C) 2015 Toppano <https://github.com/sukinull/libwebrtc> */
#ifndef _WEBRTC_WRAPPER_RTC_DATA_CHANNEL_H_
#define _WEBRTC_WRAPPER_RTC_DATA_CHANNEL_H_
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

	struct datachannel_ctx_vars_;
	struct datachannel_ctx_ops_;

	typedef struct datachannel_ctx_ {
		struct datachannel_ctx_vars_ * vars;
		struct datachannel_ctx_ops_ * ops;
	}datachannel_ctx;


	typedef void(*on_datachennel_open_callback)();
	typedef void(*on_datachannel_close_callback)();
	typedef void(*on_datachannel_message_callback)();
	typedef void(*on_datachannel_error_callback)(const char *);

	WEBRTC_WRAPPER_API const char* label(); // readonly attribute DOMString label
	
	WEBRTC_WRAPPER_API int datachannel_close(datachannel_ctx *ctx);
	WEBRTC_WRAPPER_API int datachannel_send(datachannel_ctx *ctx, /*TODO:_Buffer buf,*/ int binary);


#ifdef __cplusplus
}
#endif
#endif // _WEBRTC_WRAPPER_RTC_DATA_CHANNEL_H_