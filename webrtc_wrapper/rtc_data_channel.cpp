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
	on_datachennel_open_callback on_open_cb;
	on_datachannel_message_callback on_message_cb;
}datachannel_ctx_ops_;