CC=gcc
CFLAGS=-Wall -Wformat=0 -lcrypt -g -pthread
ODIR=obj
IDIR=../include
LDIR=../lib
PROGRAM=id_echo_sv id_echo_cl is_echo_sv is_echo_cl is_echo_inetd_sv

_DEPS = 
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = error_functions.o get_num.o inet_sockets.o become_daemon.o
OBJ = $(patsubst %, $(ODIR)/%,$(_OBJ))

all : $(PROGRAM)

$(PROGRAM): $(patsubst %, $(ODIR)/%,$(addsuffix .o,$(PROGRAM))) $(OBJ)
	$(CC) -o $@ $(ODIR)/$@.o $(OBJ) $(CFLAGS)

$(ODIR)/%.o: $(LDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o $(PROGRAM) *.o