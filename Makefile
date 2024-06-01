CC=gcc
CFLAGS=-march=native -O2 -pipe -I.
EXECUTABLE=systeminfo
LIBS=-lm
$(EXECUTABLE): main.o extra_func.o
	$(CC) -o $(EXECUTABLE) main.o extra_func.o $(LIBS)

main.o: main.c main.h
	$(CC) -c -o main.o main.c $(CFLAGS)

extra_func.o: extra_func.c main.h
	$(CC) -c -o extra_func.o extra_func.c $(CFLAGS)

clean:
	rm -f *.o $(EXECUTABLE)
