#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <lttng/ust-events.h>
#include <lttng/tracepoint.h>
#include <lttng/ringbuffer-config.h>

#define NB_CHAMP 3
void add_int_event_field( struct lttng_event_field *field, char *name)
{
	struct lttng_event_field f = {
		.name = name,
		.type = __type_integer(int, LITTLE_ENDIAN, 10, none),
		.nowrite = 0
	};

	memcpy(field, &f, sizeof(struct lttng_event_field));
}

void add_float_event_field( struct lttng_event_field *field, char *name)
{
	struct lttng_event_field f = {
		.name = name,
		.type = __type_float(float),
		.nowrite = 0
	};

	memcpy(field, &f, sizeof(struct lttng_event_field));
}
void add_char_event_field( struct lttng_event_field *field, char *name)
{
	struct lttng_event_field f = {
		.name = name,
		.type = __type_integer(char, LITTLE_ENDIAN, 10, none),
		.nowrite = 0
	};

	memcpy(field, &f, sizeof(struct lttng_event_field));
}

void fake_probe(struct lttng_event* __tp_data)
{}
int main (int argc, const char* argv[]) 
{
	char *name = malloc(sizeof(char) *50);
	char *sign = malloc(sizeof(char) *50);
	sprintf(name,"test:testevent");
	sprintf(sign,"testevent");

	int i, j;
	int champs0 = 18;
	float champs1 = 42.212;
	char champs2 = 'f';

	char *nom_champs[NB_CHAMP];
	nom_champs[0] = "int1";
	nom_champs[1] = "float2";
	nom_champs[2] = "char3";

	int type[3];
	type[0] = 0;
	type[1] = 1;
	type[2] = 2;

	size_t size_champs[NB_CHAMP];
	size_champs[0] = sizeof(int);
	size_champs[1] = sizeof(float);
	size_champs[2] = sizeof(char);

	//Tracepoint
	struct tracepoint t= {
		.name = name,
		.state = 0,
		.probes = NULL,
		.tracepoint_provider_ref = NULL,
		.signature = sign,
	};
	struct tracepoint *tp = malloc(sizeof(struct tracepoint));
	memcpy(tp, &t, sizeof(struct tracepoint));

	//const struct tracepoint *test_tracepoints[] = {&t};
	struct tracepoint **test_tracepoint = malloc(sizeof(struct tracepoint*));
	test_tracepoint[0] = &t;
	tracepoint_register_lib(test_tracepoint, NB_CHAMP);

	//Event fields
	struct lttng_event_field *event_fields = malloc(sizeof(struct lttng_event_field)*NB_CHAMP);
	add_int_event_field(&event_fields[0], nom_champs[0]);
	add_float_event_field(&event_fields[1], nom_champs[1]);
	add_char_event_field(&event_fields[2], nom_champs[2]);

	//Event description
	struct lttng_event_desc event_desc_1 = {
		.name = name,
		.probe_callback = (void (*)()) fake_probe ,
		.ctx = NULL ,
		.fields = (const struct lttng_event_field *) event_fields,
		.nr_fields = NB_CHAMP,
		.signature = sign,
	};
	struct lttng_event_desc *event_d = malloc(sizeof(struct lttng_event_desc));
	memcpy(event_d, &event_desc_1, sizeof(struct lttng_event_desc));

	struct lttng_event_desc **event_desc = malloc(sizeof(struct lttng_event_desc *));
	//memcpy(event_desc[0], event_d, sizeof(struct lttng_event_desc));
	event_desc[0] = event_d;
	//probe description
	struct lttng_probe_desc desc = {
		.provider = "test",
		.event_desc = (const struct lttng_event_desc **)event_desc,
		.nr_events = 1,
		.head = { NULL, NULL },
		.lazy_init_head = { NULL, NULL },
		.lazy = 0,
		.major = LTTNG_UST_PROVIDER_MAJOR,
		.minor = LTTNG_UST_PROVIDER_MINOR,
	};

	struct lttng_probe_desc *probe_desc = malloc(sizeof(struct lttng_probe_desc));
	memcpy(probe_desc, &desc, sizeof(struct lttng_probe_desc));
	lttng_probe_register(probe_desc);

	for (i = 0; i < 10; ++i)
	{
		struct tracepoint_probe *__tp_probe;

		__tp_probe = test_tracepoint[0]->probes;
		void *__tp_data =__tp_probe->data;

		struct lttng_event *__event = (struct lttng_event *) __tp_data;
		struct lttng_channel *__chan = __event->chan;
		struct lttng_ust_lib_ring_buffer_ctx __ctx;
		size_t __event_len, __event_align;
		int __ret;

    		__event_len = 0;
		for(j = 0; j < NB_CHAMP; ++j)
		{
			switch(type[j]){
				case 0:
					__event_len += lib_ring_buffer_align(__event_len, lttng_alignof(int));
					__event_len += sizeof(int);
					break;
				case 1:
					__event_len += lib_ring_buffer_align(__event_len, lttng_alignof(float));
					__event_len += sizeof(float);
					break;
				case 2:
					__event_len += lib_ring_buffer_align(__event_len, lttng_alignof(char));
					__event_len += sizeof(char);
					break;
				default:
					exit(-1);
			}
		}
		__event_align = 0;
		lib_ring_buffer_ctx_init(&__ctx, __chan->chan, __event, __event_len,
				__event_align, -1, __chan->handle);
		__ctx.ip = __builtin_return_address(0);
		__ret = __chan->ops->event_reserve(&__ctx, __event->id);
		if (__ret < 0)
			return 0;
		for(j = 0; j < NB_CHAMP; ++j)
		{
			switch(type[j]){
				case 0:
					__chan->ops->event_write(&__ctx, &(champs0), sizeof(int));
					lib_ring_buffer_align(__event_len, lttng_alignof(int));
					break;
				case 1:
					__chan->ops->event_write(&__ctx, &(champs1), sizeof(float));
					lib_ring_buffer_align(__event_len, lttng_alignof(float));
					break;
				case 2:
					__chan->ops->event_write(&__ctx, &(champs2), sizeof(char));
					lib_ring_buffer_align(__event_len, lttng_alignof(char));
					break;
				default:
					exit(-1);
			}
		}

		__chan->ops->event_commit(&__ctx);
	}
	return 0;
}
