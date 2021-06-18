CC=gcc
CFLAGS=-Wall -Wformat=0 -lcrypt -g -pthread
ODIR=obj
IDIR=../include
LDIR=../lib
PROGRAM=daemon_SIGHUP

_DEPS = tlpi_hdr.h get_num.h error_functions.h signal_functions.h become_daemon.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = error_functions.o get_num.o signal_functions.o become_daemon.o $(PROGRAM).o
OBJ = $(patsubst %, $(ODIR)/%,$(_OBJ))

$(PROGRAM): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(ODIR)/get_num.o: $(LDIR)/get_num.c $(IDIR)/get_num.h 
	$(CC) -c -o $@ $(LDIR)/get_num.c $(CFLAGS)
$(ODIR)/error_functions.o: $(LDIR)/error_functions.c $(IDIR)/error_functions.h  $(IDIR)/tlpi_hdr.h 
	$(CC) -c -o $@ $(LDIR)/error_functions.c $(CFLAGS)
$(ODIR)/signal_functions.o: $(LDIR)/signal_functions.c $(IDIR)/signal_functions.h  $(IDIR)/tlpi_hdr.h 
	$(CC) -c -o $@ $(LDIR)/signal_functions.c $(CFLAGS)
$(ODIR)/become_daemon.o: $(LDIR)/become_daemon.c $(IDIR)/become_daemon.h  $(IDIR)/tlpi_hdr.h 
	$(CC) -c -o $@ $(LDIR)/become_daemon.c $(CFLAGS)


$(ODIR)/$(PROGRAM).o: $(PROGRAM).c $(IDIR)/tlpi_hdr.h 
	$(CC) -c -o $@ $(PROGRAM).c $(CFLAGS) 

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o $(PROGRAM) *.o