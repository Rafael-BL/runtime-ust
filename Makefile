CFLAGS= -g -O0 -Wall 

all: mutator mutatee tp.so

mutator: mutator.cpp 
	g++ $(CFLAGS)  $^ -llttng-ust -llttng-ust-tracepoint -ldyninstAPI -o $@
mutatee: mutatee.c
	gcc $(CFLAGS) $^ -o $@
tp.so: tp.o
	g++ -shared  -fPIC -DPIC $^ -ldl -llttng-ust -o $@

tp.o: tp.cpp 
	g++ -Wall -I. -fno-strict-aliasing -Wall -g -c $< -fPIC -DPIC -o $@

clean:
	rm -f mutator mutatee tp.o tp.so
run: all
	lttng create
	lttng enable-event -a -u
	lttng start
	env DYNINSTAPI_RT_LIB=/usr/lib/libdyninstAPI_RT.so ./mutator
	lttng stop
	lttng view
	lttng destroy
