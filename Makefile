LD_FLAGS = -lz

all: deflate inflate

deflate: deflate.c
	gcc $(LD_FLAGS) -o deflate deflate.c

inflate: inflate.c
	gcc $(LD_FLAGS) -o inflate inflate.c

clean:
	rm -f deflate inflate *.o
