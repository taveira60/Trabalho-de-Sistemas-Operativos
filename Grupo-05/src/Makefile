CC      := gcc
CFLAGS  := -Wall -Wextra -g

# headers comuns
DEPS    := meta.h 

# objetos “biblioteca” comum
OBJ_LIB := meta.o 

# objetos de cada binário
OBJ_CLIENT  := dclient.o $(OBJ_LIB)
OBJ_SERVER  := dservidor.o $(OBJ_LIB)

# mkfifo é um programa separado (porque mkfifo.c tem main)
OBJ_FIFO    := mkfifo.o

# alvo padrão compila tudo
all: dclient dservidor mkfifo

# cliente
dclient: $(OBJ_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^

# servidor
dservidor: $(OBJ_SERVER)
	$(CC) $(CFLAGS) -o $@ $^

# utilitário mkfifo
mkfifo: $(OBJ_FIFO)
	$(CC) $(CFLAGS) -o $@ $^

# regra genérica: .c -> .o
# inclui dependências de headers comuns
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# limpa tudo
clean:
	rm -f *.o dclient dservidor mkfifo

.PHONY: all clean
