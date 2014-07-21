#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <lttng/ust-events.h>
#include <lttng/tracepoint.h>
#include <lttng/ringbuffer-config.h>

#include <BPatch.h>
#include <BPatch_function.h>
extern "C" int tracepoint_register_lib(struct tracepoint * const *tracepoints_start, int nb);

extern "C" {
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
}
void fake_probe(struct lttng_event* __tp_data)
{}
using namespace std;
int main (int argc, const char* argv[]) 
{
	BPatch bpatch;
	//BPatch_addressSpace *handle = bpatch.processCreate("./mutatee", NULL);
	BPatch_addressSpace *handle = bpatch.openBinary("./mutatee", NULL);
	BPatch_image *image = handle->getImage();
	vector<BPatch_function*> functions;
	image->findFunction("getAnswer", functions);

	vector<BPatch_localVar *> *params = functions[0]->getParams();
	int nb_field = params->size();

	char *name = (char*)malloc(sizeof(char) *50);
	char *sign = (char*)malloc(sizeof(char) *50);
	sprintf(name,"prvdr:event");
	sprintf(sign,"event");

	int i, j;

	char **fieldNames = (char**) malloc(sizeof(char *) *nb_field );
	size_t *size_champs = (size_t *) malloc(sizeof(size_t)*nb_field);
	for(int i = 0;i < nb_field ; ++i)
	{
		fieldNames[i] = (char*) malloc(sizeof(char) *50);
		strncpy(fieldNames[i], (*params)[i]->getName(), 50);
		size_champs[i] = (*params)[i]->getType()->getSize();
	}

	//Tracepoint
	struct tracepoint t= {
		.name = name,
		.state = 0,
		.probes = NULL,
		.tracepoint_provider_ref = NULL,
		.signature = sign,
	};
	struct tracepoint *tp = (struct tracepoint*) malloc(sizeof(struct tracepoint));
	memcpy(tp, &t, sizeof(struct tracepoint));

	//const struct tracepoint *test_tracepoints[] = {&t};
	struct tracepoint **test_tracepoint = (struct tracepoint**) malloc(sizeof(struct tracepoint*));
	test_tracepoint[0] = &t;
	tracepoint_register_lib(test_tracepoint, nb_field);

	//Event fields
	struct lttng_event_field *event_fields = (struct lttng_event_field* ) malloc(sizeof(struct lttng_event_field)*nb_field);

	size_t __event_len = 0;
	for(int i = 0;i < nb_field ; ++i)
	{
		// Add a field depending on the type of the parameter
		switch((*params)[i]->getType()->getDataClass())
		{
			case BPatch_dataScalar:
			{
				string typeName = (*params)[i]->getType()->getName();
				if(typeName == "char")
				{
					add_char_event_field(&event_fields[i], fieldNames[i]);
					__event_len += lib_ring_buffer_align(__event_len, lttng_alignof(char));
					__event_len += sizeof(char);
				}
				else if (typeName == "short int")
				{
				}
				else
				{
					add_int_event_field(&event_fields[i], fieldNames[i]);
					__event_len += lib_ring_buffer_align(__event_len, lttng_alignof(int));
					__event_len += sizeof(int);
				}
				break;
			}
			default:
			{
				cout<<"Dataclass unsupported"<<endl;
				break;
			}
		}
	}

	//Event description
	struct lttng_event_desc event_desc_1 = {
		.name = name,
		.probe_callback = (void (*)()) fake_probe ,
		.ctx = NULL,
		.fields = (const struct lttng_event_field *) event_fields,
		.nr_fields =(unsigned int) nb_field,
		.loglevel = NULL,
		.signature = sign,
	};
	struct lttng_event_desc *event_d = (struct lttng_event_desc *) malloc(sizeof(struct lttng_event_desc));
	memcpy(event_d, &event_desc_1, sizeof(struct lttng_event_desc));

	struct lttng_event_desc **event_desc = (struct lttng_event_desc **) malloc(sizeof(struct lttng_event_desc *));
	//memcpy(event_desc[0], event_d, sizeof(struct lttng_event_desc));
	event_desc[0] = event_d;
	//probe description
	struct lttng_probe_desc desc = {
		.provider = "prvdr",
		.event_desc = (const struct lttng_event_desc **)event_desc,
		.nr_events = 1,
		.head = { NULL, NULL },
		.lazy_init_head = { NULL, NULL },
		.lazy = 0,
		.major = LTTNG_UST_PROVIDER_MAJOR,
		.minor = LTTNG_UST_PROVIDER_MINOR,
	};

	struct lttng_probe_desc *probe_desc = (struct lttng_probe_desc *) malloc(sizeof(struct lttng_probe_desc));
	memcpy(probe_desc, &desc, sizeof(struct lttng_probe_desc));
	lttng_probe_register(probe_desc);

	for (i = 0; i < 10; ++i)
	{
		void *__tp_data = test_tracepoint[0]->probes->data;
		struct lttng_event *__event = (struct lttng_event *) __tp_data;
		struct lttng_channel *__chan = __event->chan;
		struct lttng_ust_lib_ring_buffer_ctx __ctx;
		size_t __event_align;
		int __ret;

		__event_align = 0;
		//Must be done in the mutatee address space
		lib_ring_buffer_ctx_init(&__ctx, __chan->chan, __event, __event_len,
				__event_align, -1, __chan->handle);
		__ctx.ip = __builtin_return_address(0);
		__ret = __chan->ops->event_reserve(&__ctx, __event->id);
		if (__ret < 0)
			return 0;
		for(j = 0; j < nb_field; ++j)
		{
			string typeName = (*params)[j]->getType()->getName();
			if(typeName == "char")
			{
				__chan->ops->event_write(&__ctx, &j, sizeof(char));
				lib_ring_buffer_align(__event_len, lttng_alignof(char));
			}
			else
			{
				__chan->ops->event_write(&__ctx, &j, sizeof(int));
				lib_ring_buffer_align(__event_len, lttng_alignof(int));
			}
		}

		__chan->ops->event_commit(&__ctx);
	}
	return 0;
}
