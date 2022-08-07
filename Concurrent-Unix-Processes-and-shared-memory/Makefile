CC=gcc
OBJ = logParse.o
CFILE = logParse.c
MATH = -lm
Cleanup=rm -rf *.o logParse
EXE = logParse
WITHNAME = -o
CFLAGS=-c

.SUFFIXES: .c .o

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(WITHNAME) $@ $(OBJ) $(MATH)

.c.o: 
	$(CC) $(CFLAGS) $(MATH) $<

clean:
	$(Cleanup)
