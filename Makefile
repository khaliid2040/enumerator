CC=gcc
EXECUTABLE=systeminfo
INCLUDE_DIR = .
ifneq ($(wildcard config.mk),)
    include config.mk
endif	
$(EXECUTABLE): main.o extra_func.o storage.o memory.o cpuinfo.o process.o network.o route.o arp.o system.o security.o
	$(CC) -o $(EXECUTABLE) main.o extra_func.o storage.o memory.o cpuinfo.o process.o network.o route.o arp.o system.o security.o $(LDFLAGS)

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
system.o:
	$(CC) -c -o system.o extra/system.c $(CFLAGS) -I$(INCLUDE_DIR)
network.o:
	$(CC) -c -o network.o net/network.c $(CFLAGS) -I$(INCLUDE_DIR)
route.o:
	$(CC) -c -o route.o net/route.c $(CFLAGS) -I$(INCLUDE_DIR)
arp.o:
	$(CC) -c -o arp.o net/arp.c $(CFLAGS) -I$(INCLUDE_DIR)
security.o:
	$(CC) -c -o security.o extra/security.c $(CFLAGS) -I$(INCLUDE_DIR) $(LIBS	)
install:
	@echo "Checking .local"
	@if [ -d ~/.local/bin ]; then\		
		mv -v systeminfo /home/khaalid/.local/bin
	@else\
		mkdir -pv ~/.local/bin
		mv -v systeminfo ~/.local/bin
	fi	
checkdep:
	@./config.sh
clean:
	rm -f *.o $(EXECUTABLE) config.mk
