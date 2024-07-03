CC=gcc
CFLAGS=-march=native -O2 -pipe -I.
EXECUTABLE=systeminfo
LIBS=-lm
INCLUDE_DIR = .

$(EXECUTABLE): main.o extra_func.o storage.o memory.o cpuinfo.o process.o network.o route.o
	$(CC) -o $(EXECUTABLE) main.o extra_func.o storage.o memory.o cpuinfo.o process.o network.o route.o $(LIBS)

main.o: main.c main.h
	$(CC) -c -o main.o main.c $(CFLAGS) -I$(INCLUDE_DIR)

extra_func.o: extra/extra_func.c main.h
	$(CC) -c -o extra_func.o extra/extra_func.c $(CFLAGS) -I$(INCLUDE_DIR)

storage.o: extra/storage.c main.h
	$(CC) -c -o storage.o extra/storage.c $(CFLAGS) -I$(INCLUDE_DIR)
memory.o: extra/memory.c main.h
	$(CC) -c -o memory.o extra/memory.c $(CFLAGS) -I$(INCLUDE_DIR)
cpuinfo.o:
	$(CC) -c -o cpuinfo.o extra/cpuinfo.c $(CFLAGS) -I$(INCLUDE_DIR)
process.o:
	$(CC) -c -o process.o extra/process.c $(CFLAGS) -I$(INCLUDE_DIR)
network.o:
	$(CC) -c -o network.o net/network.c $(CFLAGS) -I$(INCLUDE_DIR)
route.o:
	$(CC) -c -o route.o net/route.c $(CFLAGS) -I$(INCLUDE_DIR)
install:
	mv -v systeminfo /home/khaalid/.local/bin
clean:
	rm -f *.o $(EXECUTABLE)
