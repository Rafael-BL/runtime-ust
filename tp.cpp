#include <lttng/ust-events.h>
#include <lttng/tracepoint.h>
#include <lttng/ringbuffer-config.h>

#include <BPatch.h>
#include <BPatch_function.h>

#include "buche/buche.h"

void fake_probe(struct lttng_event* __tp_data)
{}

void tp_int(struct tracepoint *t, int event_len, int value )
{
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;
	struct lttng_ust_lib_ring_buffer_ctx __ctx;
	size_t __event_align;
	int __ret;

	__event_align = 0;
	lib_ring_buffer_ctx_init(&__ctx, __chan->chan, __event, event_len,
			__event_align, -1, __chan->handle);
	__ctx.ip = __builtin_return_address(0);
	__ret = __chan->ops->event_reserve(&__ctx, __event->id);
	if (__ret < 0)
		return;
	__chan->ops->event_write(&__ctx, &value, sizeof(int));
	lib_ring_buffer_align(event_len, lttng_alignof(int));

	__chan->ops->event_commit(&__ctx);
}

//static struct lttng_ust_lib_ring_buffer_ctx static_ctx;
void update_event_len_int(unsigned int *event_len, int value)
{

	*event_len += lib_ring_buffer_align(*event_len, lttng_alignof(int));
	*event_len += sizeof(int);
	BUCHE("z31 %d", *event_len);
}

void update_event_len_char(unsigned int *event_len, char value)
{
	*event_len += lib_ring_buffer_align(*event_len, lttng_alignof(char));
	*event_len += sizeof(char);
	BUCHE("z2 %d", *event_len);
}

void update_event_len_char_ptr(unsigned int *event_len, char *value)
{
	*event_len += lib_ring_buffer_align(*event_len, lttng_alignof(unsigned int));
	*event_len += sizeof(unsigned int) ;
	*event_len += lib_ring_buffer_align(*event_len, lttng_alignof(char));
	*event_len += (sizeof(char) * (strlen(value) + 1));
	BUCHE("z1 %d", *event_len);
}

void init_ctx( struct lttng_ust_lib_ring_buffer_ctx *static_ctx, 
			struct tracepoint *t, unsigned int *event_len, int isEnable)
{
	BUCHE("a1 %d", *event_len);
	if(!isEnable)
	{
		return;
	}
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;
	size_t __event_align;

	__event_align = 0;
	lib_ring_buffer_ctx_init(static_ctx, __chan->chan, __event, *event_len,
			__event_align, -1, __chan->handle);
	static_ctx->ip = __builtin_return_address(0);
	__chan->ops->event_reserve(static_ctx, __event->id);
}

void event_write_int( struct lttng_ust_lib_ring_buffer_ctx *static_ctx,
			struct tracepoint *t, unsigned int *event_len, int value, int isEnable)
{
	BUCHE("a2");
	if(!isEnable)
	{
		return;
	}
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	lib_ring_buffer_align(*event_len, lttng_alignof(int));
	__chan->ops->event_write(static_ctx, &value, sizeof(int));
}

void event_write_char( struct lttng_ust_lib_ring_buffer_ctx *static_ctx,
			struct tracepoint *t, unsigned int *event_len, char value, int isEnable)
{
	BUCHE("a3");
	if(!isEnable)
	{
		return;
	}
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	lib_ring_buffer_align(*event_len, lttng_alignof(char));
	__chan->ops->event_write(static_ctx, &value, sizeof(char));
}

void event_write_char_ptr( struct lttng_ust_lib_ring_buffer_ctx *static_ctx,
			struct tracepoint *t, unsigned int *event_len, char* value, int isEnable)
{
	BUCHE("a3 %s", value);
	if(!isEnable)
	{
		return;
	}
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	unsigned int len = strlen(value)+1;
	lib_ring_buffer_align(*event_len, lttng_alignof(unsigned int));
	__chan->ops->event_write(static_ctx, &len, sizeof(unsigned int));

	lib_ring_buffer_align(*event_len, lttng_alignof(char));
	__chan->ops->event_write(static_ctx, value, sizeof(char) * len);
}


void event_write_float( struct lttng_ust_lib_ring_buffer_ctx *static_ctx,
			struct tracepoint *t, unsigned int *event_len, float value, int isEnable)
{
	BUCHE("a4 %f", value);
	if(!isEnable)
	{
		return;
	}
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	float a = (float) value;

	lib_ring_buffer_align(*event_len, lttng_alignof(float));
	__chan->ops->event_write(static_ctx, &a, sizeof(float));
}

void event_commit( struct lttng_ust_lib_ring_buffer_ctx *static_ctx,
			struct tracepoint *t, int isEnable)
{
	BUCHE("a5");
	if(!isEnable)
	{
		return;
	}

	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	__chan->ops->event_commit(static_ctx);
}
