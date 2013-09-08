SRC = src
CC = gcc
CFLAGS = -Wall -Wextra -Os
ODIR = obj
LIBS = -lrt
_DEPS = am335x.h gpio.h nibble.h fwh.h lpc.h proto.h pm49fl00x.h
DEPS = $(patsubst %,$(SRC)/%,$(_DEPS))
_OBJ = gpio.o nibble.o fwh.o lpc.o proto.o pm49fl00x.o bbflash.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SRC)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bbflash: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o bbflash
