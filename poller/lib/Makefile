
all: stress

libs: libio_ring.a

stress: main.o libio_ring.a
	gcc -lm -o stress main.o -L. -lio_ring

main.o: main.c
	gcc -O -c main.c

libio_ring.a: io_ring.o
	ar rcs libio_ring.a io_ring.o

io_ring.o: io_ring.c
	gcc -O -c io_ring.c -o io_ring.o
clean:
	rm -f *.a *.o
