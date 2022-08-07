CC=gcc
OBJ_1 = oss.o
OBJ_2 = user.c
CFILE = oss.c
CFILE_2 = user.c
MATH = -lm
Cleanup=rm -rf *.o oss user
EXE = $(EXE_1) $(EXE_2)
EXE_1 = oss
EXE_2 = user
WITHNAME = -o
CFLAGS= -Wall -c
SEM = -lpthread

.SUFFIXES: .c .o

all: $(EXE)

$(EXE_1): $(OBJ_1)
	$(CC) $(WITHNAME) $@ $(OBJ_1) $(MATH) $(SEM)

$(EXE_2): $(OBJ_2) 
	$(CC) $(WITHNAME) $@ $(OBJ_2) $(MATH) $(SEM)

.c.o: 
	$(CC) $(CFLAGS) $(MATH) $(SEM) $<

clean:
	$(Cleanup)
