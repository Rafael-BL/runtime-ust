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
#include <BPatch_point.h>

#include "buche/buche.h"
#define MAX_STR_LEN 20
#define TARGETED_FCT "toronto"
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
using namespace std;
void register_tp_from_mutatee(BPatch_process *handle, BPatch_variableExpr *tp)
{
	BUCHE("Registering tracepoint from mutatee");
	vector<BPatch_function*> functions;
	BPatch_image *image = handle->getImage();
	std::vector<BPatch_snippet *> args;

	image->findFunction("tracepoint_register", functions);

	args.push_back(new BPatch_constExpr( tp->getBaseAddr()));
	BPatch_funcCallExpr tp_register_lib_call(*functions[0], args);
	handle->oneTimeCode(tp_register_lib_call);
}
void probe_register_from_mutatee(BPatch_process *handle, BPatch_variableExpr *probe)
{
	BUCHE("Registering probe from mutatee");
	vector<BPatch_function*> functions;
	BPatch_image *image = handle->getImage();
	std::vector<BPatch_snippet *> args;

	image->findFunction("lttng_probe_register", functions);

	args.push_back(new BPatch_constExpr( probe->getBaseAddr()));
	BPatch_funcCallExpr probe_register_call(*functions[0], args);
	handle->oneTimeCodeAsync(probe_register_call);
}

int main (int argc, const char* argv[])
{
	BPatch bpatch;
	bpatch.setDebugParsing(false);
	bpatch.setDelayedParsing(true);

	//BPatch_process *handle = bpatch.processCreate("./mutatee", NULL );
	BUCHE("Fork the mutatee process");
	int mutateePid = fork();

    	if(mutateePid == 0)
    	{
		char *args[4] = {NULL};
		char *envs[3] = {NULL};
		args[0] = (char *) "./mutatee";

		envs[0] = (char *) "LD_PRELOAD=/usr/local/lib/liblttng-ust.so";
		envs[1] = (char *) "HOME=/home/frdeso";
		execve("./mutatee", args, envs);
    	}

    	sleep(1);
	BUCHE("Attaching to the mutatee process");
    	BPatch_process *handle = bpatch.processAttach(NULL, mutateePid);

	BUCHE("Loading library in mutatee addr. space");
	handle->loadLibrary("./tp.so");
	handle->loadLibrary("/usr/local/lib/liblttng-ust.so");
	handle->stopExecution();

	BPatch_image *image = handle->getImage();
	vector<BPatch_function*> functions, tp_function, field_fct;

	BUCHE("Looking for \"%s\" function in the mutatee addr. space", TARGETED_FCT)
	image->findFunction(TARGETED_FCT, functions);

	vector<BPatch_localVar *> *params = functions[0]->getParams();
	int nb_field = params->size();

	/*TODO:Move to binary through instrumentation  */
	BPatch_variableExpr *nameExpr = handle->malloc(sizeof(char) * MAX_STR_LEN);
	char nameArr[MAX_STR_LEN] = "p:event_" TARGETED_FCT;
	nameExpr->writeValue((char*)nameArr, MAX_STR_LEN, false);

	BPatch_variableExpr *signExpr = handle->malloc(sizeof(char) * MAX_STR_LEN);
	char signArr[MAX_STR_LEN] = "event_" TARGETED_FCT;
	signExpr->writeValue((char *)signArr, MAX_STR_LEN, false);

	BPatch_variableExpr *prvdrExpr = handle->malloc(sizeof(char) * MAX_STR_LEN);
	char provArr[MAX_STR_LEN] = "p";
	prvdrExpr->writeValue((char*)provArr, MAX_STR_LEN, false);

	//Tracepoint
	struct tracepoint t= {
		.name =(const char*) nameExpr->getBaseAddr(), //Does this work?
		.state = 0,
		.probes = NULL,
		.tracepoint_provider_ref = NULL,
		.signature =(const char*) signExpr->getBaseAddr(),//Does this work?
	};

	BPatch_variableExpr *tpExpr = handle->malloc(sizeof(struct tracepoint));
	tpExpr->writeValue((void *) &t, sizeof(struct tracepoint), false);

	register_tp_from_mutatee(handle, tpExpr);

	//Event fields
	struct lttng_event_field *event_fields = (struct lttng_event_field* ) malloc(sizeof(struct lttng_event_field)*nb_field);
	BPatch_variableExpr *event_fieldsExpr = handle->malloc(sizeof(struct lttng_event_field) * nb_field);

	int __event_len = 0;
	BUCHE("Generating event fields according to the function's params");
	for(int i = 0;i < nb_field ; ++i)
	{
		BPatch_variableExpr* fieldNameExpr = handle->malloc(sizeof(char) *MAX_STR_LEN);
		fieldNameExpr->writeValue((char *)(*params)[i]->getName(), MAX_STR_LEN);

		// Add a field depending on the type of the parameter
		switch((*params)[i]->getType()->getDataClass())
		{
			case BPatch_dataScalar:
			{
				string typeName = (*params)[i]->getType()->getName();
				if(typeName == "char")
				{
					add_char_event_field(&event_fields[i],(char *) fieldNameExpr->getBaseAddr());
					__event_len += lib_ring_buffer_align(__event_len, lttng_alignof(char));
					__event_len += sizeof(char);
					image->findFunction("event_write_char", field_fct);
				}
				else
				{
					add_int_event_field(&event_fields[i],(char *) fieldNameExpr->getBaseAddr());
					__event_len += lib_ring_buffer_align(__event_len, lttng_alignof(int));
					__event_len += sizeof(int);
					image->findFunction("event_write_int", field_fct);
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
	event_fieldsExpr->writeValue(event_fields, sizeof(struct lttng_event_field)*nb_field, false);

	//Event description
	struct lttng_event_desc event_desc = {
		.name = (const char*) nameExpr->getBaseAddr(),
		.probe_callback = (void (*)()) 1337, //FIXME: must set the probe callback to none null value but is not used
		.ctx = NULL,
		.fields = (const struct lttng_event_field *) event_fieldsExpr->getBaseAddr(),
		.nr_fields = (unsigned int) nb_field,
		.loglevel = NULL,
		.signature = (const char*) signExpr->getBaseAddr(),
	};

	BPatch_variableExpr *event_descExpr = handle->malloc(sizeof(struct lttng_event_desc));
	event_descExpr->writeValue(&event_desc, sizeof(struct lttng_event_desc), false);

	BPatch_variableExpr *event_descArrayExpr = handle->malloc(sizeof(struct lttng_event_desc*));
	unsigned long addr =(unsigned long) event_descExpr->getBaseAddr();
	event_descArrayExpr->writeValue(&addr,  sizeof(struct lttng_event_desc*), false);

	//probe description
	struct lttng_probe_desc desc = {
		.provider = (const char*) prvdrExpr->getBaseAddr(),
		.event_desc = (const struct lttng_event_desc **) event_descArrayExpr->getBaseAddr(),
		.nr_events = 1,
		.head = { NULL, NULL },
		.lazy_init_head = { NULL, NULL },
		.lazy = 0,
		.major = LTTNG_UST_PROVIDER_MAJOR,
		.minor = LTTNG_UST_PROVIDER_MINOR,
	};
	BPatch_variableExpr *probe_descExpr = handle->malloc(sizeof(struct lttng_probe_desc));
	probe_descExpr->writeValue(&desc, sizeof(struct lttng_probe_desc), false);

	probe_register_from_mutatee(handle, probe_descExpr);

	BUCHE("Preparing arguments for tracepoint calling");
	std::vector<BPatch_snippet *> args;
	std::vector<BPatch_function *> init_ctx_fct, commit_fct;

	std::vector<BPatch_snippet *> call_sequence;

	BUCHE("\tAllocate ctx data struct. in the mutatee");
	BPatch_variableExpr *ctxExpr = handle->malloc(sizeof(struct lttng_ust_lib_ring_buffer_ctx));

	BUCHE("\tInitialize ctx data struct. in the mutatee");
	image->findFunction("init_ctx", init_ctx_fct);
	args.push_back(new BPatch_constExpr(ctxExpr->getBaseAddr()));
	args.push_back(new BPatch_constExpr(tpExpr->getBaseAddr()));
	args.push_back(new BPatch_constExpr( __event_len ));
	BPatch_funcCallExpr init_ctx_fct_call(*(init_ctx_fct[0]), args);
	call_sequence.push_back(&init_ctx_fct_call);

	args.clear();
	BUCHE("\tIterate on every params and prepare the field write call expr");

	for(int i = 0 ; i < nb_field ; ++i)
	{
		BUCHE("\t\tAdd call expr for param. %d named \"%s\"", i, (*params)[i]->getName());
		args.push_back(new BPatch_constExpr(ctxExpr->getBaseAddr()));
		args.push_back(new BPatch_constExpr(tpExpr->getBaseAddr()));
		args.push_back(new BPatch_constExpr( __event_len ));
		args.push_back(new BPatch_paramExpr(i));
		BPatch_funcCallExpr *field_call = new BPatch_funcCallExpr(*(field_fct[i]), args);
		call_sequence.push_back(field_call);

		field_fct.clear();
		args.clear();
	}

	//Find function that commits the event
	BUCHE("\tCall the event commit function");
	args.push_back(new BPatch_constExpr(ctxExpr->getBaseAddr()));
	args.push_back(new BPatch_constExpr(tpExpr->getBaseAddr()));
	image->findFunction("event_commit", commit_fct);
	BPatch_funcCallExpr event_commit_call(*(commit_fct[0]), args);
	call_sequence.push_back(&event_commit_call);
	vector<BPatch_point*> * points = functions[0]->findPoint(BPatch_entry);

	BUCHE("Add tracepoint point function call in the mutatee");
	handle->insertSnippet(BPatch_sequence(call_sequence), points[0]);

	if(handle->isStopped())
	{
		BUCHE("Continuing mutatee's execution...");
		handle->continueExecution();
	}
	if(handle->isTerminated())
		BUCHE("Mutatee dead on arrival"); //the process is supposed to be in stopped state
	while (!handle->isTerminated())
		bpatch.waitForStatusChange();

	return 0;
}
