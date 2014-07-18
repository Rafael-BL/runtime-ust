CFLAGS= -g -O0 

all: mutator mutatee

mutator: mutator.cpp 
	g++ $(CFLAGS)  $^ -llttng-ust -llttng-ust-tracepoint -o $@
mutatee: mutatee.c
	gcc $(CFLAGS) $^ -o $@
clean:
	rm -f mutator mutatee 
run: mutator
	lttng create
	lttng enable-event -a -u
	lttng start
	env LD_PRELOAD=/usr/local/lib/liblttng-ust.so ./mutator
	lttng stop
	lttng view
	lttng destroy
