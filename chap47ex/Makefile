CC=gcc
CFLAGS=-Wall -Wformat=0 -lcrypt -g -pthread -lcap
ODIR=obj
IDIR=../include
LDIR=../lib
PROGRAM=svsem_demo svsem_mon svsem_setall svsem_op

_DEPS = 
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = error_functions.o get_num.o curr_time.o
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

.PHONY: cleanall
cleanall:
	rm -f $(ODIR)/*.o $(PROGRAM) *.o *.c