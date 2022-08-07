CC=gcc
MASTER_CFILE = oss.c
CHILD_CFILE = user.c

MASTER_OBJ=$(MASTER_CFILE:.c=.o)
CHILD_OBJ=$(CHILD_CFILE:.c=.o)

MASTER_EXE = oss
CHILD_EXE = user

CFLAGS = -g -Wall
MATH = -lm

HEADER_FILE = shared_mem.h

SHARE_OBJ = clock.o helpers.o message_queue.o shared_memory.o queue.o bankers.o resource.o
SHARE_HEAD = clock.h global_constants.h helpers.h message_queue.h shared_memory.h queue.h bankers.h resource.h


%.o: %.c $(SHARE_HEAD)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(MASTER_EXE) $(CHILD_EXE)

$(MASTER_EXE): $(MASTER_OBJ) $(SHARE_OBJ)
	gcc -o $(MASTER_EXE) $^ $(CFLAGS)
	
$(CHILD_EXE): $(CHILD_OBJ) $(SHARE_OBJ)
	gcc -o $(CHILD_EXE) $^ $(CFLAGS)

clean:
	rm $(MASTER_EXE) $(MASTER_OBJ) $(CHILD_EXE) $(CHILD_OBJ) $(SHARE_OBJ)

