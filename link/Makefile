SOURCEDYNAMIC=${shell find -type f -name "dynamic_lib.so"}
all: link.o static_lib.a dynamic_lib.so dynamic_linked_lib.so
	gcc -o link link.o -L. static_lib.a -ldl $(SOURCEDYNAMIC) -Wl,-rpath,.

run:
	LD_PRELOAD_PATH= ./link

link.o: link.c
	gcc -c link.c

static_lib.a: static_lib.o
	ar cr static_lib.a static_lib.o

static_lib.o: static_lib.c
	gcc -c static_lib.c

dynamic_lib.so: dynamic_lib.o
	gcc -shared -o dynamic_lib.so dynamic_lib.o

dynamic_lib.o: dynamic_lib.c
	gcc -c -fPIC dynamic_lib.c

dynamic_linked_lib.so: dynamic_linked_lib.o
	gcc -shared -o dynamic_linked_lib.so dynamic_linked_lib.o

dynamic_linked_lib.o: dynamic_linked_lib.c
	gcc -c -fPIC dynamic_linked_lib.c

clean:
	-rm -f *.o *.so *.a