CFLAGS= -g -O0 -Wall 

all: mutator mutatee

mutator: mutator.cpp 
	g++ $(CFLAGS)  $^ -llttng-ust -llttng-ust-tracepoint -ldyninstAPI -o $@
mutatee: mutatee.c
	gcc $(CFLAGS) $^ -o $@
clean:
	rm -f mutator mutatee 
run: mutator
	lttng create
	lttng enable-event -a -u
	lttng start
	env DYNINSTAPI_RT_LIB=/usr/lib/libdyninstAPI_RT.so LD_PRELOAD=/usr/local/lib/liblttng-ust.so ./mutator
	lttng stop
	lttng view
	lttng destroy
