CC=gcc
OBJ = dt.o
CFILE = dt.c
MATH = -lm
Cleanup=rm -rf *.o dt
EXE = dt
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
