#include "common_.hpp"
#include "buffer_.hpp"
#include "rtc_data_channel.h"
using namespace std;


typedef struct datachannel_ctx_vars_
{
	char *label_;
}datachannel_ctx_vars_;

typedef struct datachannel_ctx_ops_
{
	on_datachannel_close_callback on_close_cb;
	on_datachannel_error_callback on_error_cb;
	on_datachannel_open_callback on_open_cb;
	on_datachannel_message_callback on_message_cb;
}datachannel_ctx_ops_;

datachannel_ctx *create_datachannel_ctx()
{
	datachannel_ctx *ret = (datachannel_ctx *)calloc(1, sizeof(datachannel_ctx));
	ret->ops = (datachannel_ctx_ops_ *)calloc(1, sizeof(datachannel_ctx_ops_));
	ret->vars = (datachannel_ctx_vars_ *)calloc(1, sizeof(datachannel_ctx_vars_));
	return ret;
}

void set_datachannel_cb_onopen(datachannel_ctx* ctx, on_datachannel_open_callback on_open_cb)
{
	ctx->ops->on_open_cb = on_open_cb;
}

void set_datachannel_cb_onclose(datachannel_ctx* ctx, on_datachannel_close_callback on_close_cb)
{
	ctx->ops->on_close_cb = on_close_cb;
}

void set_datachannel_cb_onmessage(datachannel_ctx* ctx, on_datachannel_message_callback on_message_cb)
{
	ctx->ops->on_message_cb = on_message_cb;
}

void set_datachannel_cb_onerror(datachannel_ctx* ctx, on_datachannel_error_callback on_error_cb)
{
	ctx->ops->on_error_cb = on_error_cb;
}