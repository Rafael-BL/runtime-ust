#include <lttng/ust-events.h>
#include <lttng/tracepoint.h>
#include <lttng/ringbuffer-config.h>

#include <BPatch.h>
#include <BPatch_function.h>

#include "../buche/buche.h"

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

static struct lttng_ust_lib_ring_buffer_ctx static_ctx;

void init_ctx(struct tracepoint *t, int event_len)
{
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;
	size_t __event_align;

	__event_align = 0;
	lib_ring_buffer_ctx_init(&static_ctx, __chan->chan, __event, event_len,
			__event_align, -1, __chan->handle);
	static_ctx.ip = __builtin_return_address(0);
	__chan->ops->event_reserve(&static_ctx, __event->id);
}

void event_write_int( struct tracepoint *t, int event_len, int value)
{
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	__chan->ops->event_write(&static_ctx, &value, sizeof(int));
	lib_ring_buffer_align(event_len, lttng_alignof(int));
}

void event_write_char( struct tracepoint *t, int event_len, char value)
{
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	__chan->ops->event_write(&static_ctx, &value, sizeof(char));
	lib_ring_buffer_align(event_len, lttng_alignof(char));
}

void event_commit( struct tracepoint *t)
{

	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	__chan->ops->event_commit(&static_ctx);
}
