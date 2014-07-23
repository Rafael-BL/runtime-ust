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

#include "../buche/buche.h"
#define MAX_STR_LEN 10
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
	BUCHE("a");
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
	BUCHE("b");
	vector<BPatch_function*> functions;
	BPatch_image *image = handle->getImage();
	std::vector<BPatch_snippet *> args;

	image->findFunction("lttng_probe_register", functions);

	args.push_back(new BPatch_constExpr( probe->getBaseAddr()));
	BPatch_funcCallExpr probe_register_call(*functions[0], args);
	handle->oneTimeCodeAsync(probe_register_call);
}
void call_printf_from_mutatee(BPatch_process *handle)
{
	BPatch_image *image = handle->getImage();
	vector<BPatch_function*> functions;
	std::vector<BPatch_snippet *> args;
	image->findFunction("printf", functions);
	BPatch_snippet *fmt = new BPatch_constExpr("bonjour********\n");
	args.push_back(fmt);
	BPatch_funcCallExpr printfCall(*(functions[0]), args);
	handle->oneTimeCodeAsync(printfCall);
}
void char_print_str(BPatch_process *handle, BPatch_snippet *var)
{
//	unsigned long addr = 0;
//	var->readValue(&addr);
//	printf("BA: %p\n", ((BPatch_constExpr*) var)->getBaseAddr());
	BPatch_image *image = handle->getImage();
	vector<BPatch_function*> functions;
	std::vector<BPatch_snippet *> args;
	image->findFunction("read_str", functions);
//	image->findFunction("printf", functions);
//	BPatch_snippet *fmt = new BPatch_constExpr("lol--->%s\n");
//	args.push_back(fmt);
	args.push_back(var);

	BPatch_funcCallExpr call(*(functions[0]), args);
	handle->oneTimeCode(call);

}
int main (int argc, const char* argv[])
{
	BPatch bpatch;

	//BPatch_process *handle = bpatch.processCreate("./mutatee", NULL );
	int mutateePid = fork();

    	if(mutateePid == 0)
    	{
		char *args[4] = {NULL};
		args[0] = (char *) "env";
		args[1] = (char *) "LD_PRELOAD=/usr/local/lib/liblttng-ust.so";
		args[2] = (char *) "./mutatee";
		execvp("./mutatee", args);
    	}

    	sleep(1);
    	BPatch_process *handle = bpatch.processAttach(NULL, mutateePid);

	BUCHE("a");
	handle->loadLibrary("./tp.so");
	BUCHE("b");
	handle->loadLibrary("/usr/local/lib/liblttng-ust.so");
	BUCHE("c");
	handle->stopExecution();
	BPatch_image *image = handle->getImage();
	vector<BPatch_function*> functions, tp_function;
	image->findFunction("quebec", functions);

	vector<BPatch_localVar *> *params = functions[0]->getParams();
	int nb_field = params->size();

	/*TODO:Move to binary through instrumentation  */
	BPatch_variableExpr *nameExpr = handle->malloc(sizeof(char) * MAX_STR_LEN);
	char nameArr[MAX_STR_LEN] = "p:event";
	nameExpr->writeValue((char*)nameArr, MAX_STR_LEN, false);

	BPatch_variableExpr *signExpr = handle->malloc(sizeof(char) * MAX_STR_LEN);
	char signArr[MAX_STR_LEN] = "event";
	signExpr->writeValue((char *)signArr, MAX_STR_LEN, false);


	BPatch_variableExpr *prvdrExpr = handle->malloc(sizeof(char) * MAX_STR_LEN);
	char provArr[MAX_STR_LEN] = "p";
	prvdrExpr->writeValue((char*)provArr, MAX_STR_LEN, false);
	//char_print_str(handle, new BPatch_constExpr( prvdrExpr->getBaseAddr()));

	BUCHE("b");
	//Tracepoint
	struct tracepoint t2= {
		.name =(const char*) nameExpr->getBaseAddr(), //Does this work?
		.state = 0,
		.probes = NULL,
		.tracepoint_provider_ref = NULL,
		.signature =(const char*) signExpr->getBaseAddr(),//Does this work?
	};

	BPatch_variableExpr *tpExpr = handle->malloc(sizeof(struct tracepoint));
	tpExpr->writeValue((void *)&t2, sizeof(struct tracepoint), false);
	BUCHE("baseaddres: %p", tpExpr->getBaseAddr());
	//BPatch_variableExpr *tpArrayExpr = handle->malloc(sizeof(struct tracepoint*) * 1);
//	unsigned long addr =(unsigned long) tpExpr->getBaseAddr();
	//tpArrayExpr->writeValue(&addr);

	register_tp_from_mutatee(handle, tpExpr);
	//Event fields
	struct lttng_event_field *event_fields = (struct lttng_event_field* ) malloc(sizeof(struct lttng_event_field)*nb_field);
	BPatch_variableExpr *event_fieldsExpr = handle->malloc(sizeof(struct lttng_event_field) * nb_field);

	int __event_len = 0;
	BUCHE("d");
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
				}
				else
				{
					add_int_event_field(&event_fields[i],(char *) fieldNameExpr->getBaseAddr());
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
	printf("event len=%d\n", __event_len);
	event_fieldsExpr->writeValue(event_fields, sizeof(struct lttng_event_field)*nb_field, false);
	long name_ptr = 0, sign_ptr = 0;
	BUCHE("e1");
	//signExpr->readValue(&sign_ptr);
	BUCHE("e2");
//	nameExpr->readValue(&name_ptr);
	BUCHE("e3");
	//Event description
	struct lttng_event_desc event_desc_1 = {
		.name = (const char*) nameExpr->getBaseAddr(),
		.probe_callback = (void (*)()) 111, //FIXME: with fake probe function in shared object
		.ctx = NULL,
		.fields = (const struct lttng_event_field *) event_fieldsExpr->getBaseAddr(),
		.nr_fields = (unsigned int) nb_field,
		.loglevel = NULL,
		.signature = (const char*) signExpr->getBaseAddr(),
	};

	BPatch_variableExpr *event_descExpr = handle->malloc(sizeof(struct lttng_event_desc));
	event_descExpr->writeValue(&event_desc_1, sizeof(struct lttng_event_desc), false);

	BPatch_variableExpr *event_descArrayExpr = handle->malloc(sizeof(struct lttng_event_desc*));
	unsigned long addr =(unsigned long) event_descExpr->getBaseAddr();
	BPatch_variableExpr *tmp = handle->malloc(1);
	event_descArrayExpr->writeValue(&addr,  sizeof(struct lttng_event_desc*), false);

	BUCHE("e");
	long provider = 0;
//	prvdrExpr->readValue(&provider);
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
	BUCHE("f");
	BUCHE("e4");
	BPatch_variableExpr *probe_descExpr = handle->malloc(sizeof(struct lttng_probe_desc));
	BUCHE("g");
	probe_descExpr->writeValue(&desc, sizeof(struct lttng_probe_desc), false);

	BUCHE("h");
	probe_register_from_mutatee(handle, probe_descExpr);
	BUCHE("i");

	std::vector<BPatch_snippet *> args;

	args.push_back(new BPatch_constExpr(tpExpr->getBaseAddr()));
	args.push_back(new BPatch_constExpr( __event_len));
	args.push_back(new BPatch_paramExpr(0));
	image->findFunction("tp_int", tp_function);
	BPatch_funcCallExpr tp_call(*(tp_function[0]), args);
	vector<BPatch_point*> * points = functions[0]->findPoint(BPatch_entry);
	handle->insertSnippet(tp_call, points[0]);

	BUCHE("j");
	if(handle->isStopped())
	{
		BUCHE("Continuing...");
		handle->continueExecution();
	}
	if(handle->isTerminated())
		BUCHE("dead");
	BUCHE("k");
	while (!handle->isTerminated())
		bpatch.waitForStatusChange();

	return 0;
}
