CC=gcc
CFLAGS=-Wall -Wformat=0 -lcrypt -g -pthread -lcap
ODIR=obj
IDIR=../include
LDIR=../lib
PROGRAM=i6d_ucase_cl i6d_ucase_sv ud_ucase_cl

_DEPS = 
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = error_functions.o get_num.o signal_functions.o 
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