# set compilator
CC=gcc
# object-making arguments
CFLAGS=-Wall -Iinclude
# obj dir
OBJDIR=obj
# objects needed
OBJ=$(OBJDIR)/swordx.o \
	$(OBJDIR)/BST.o \
	$(OBJDIR)/ThreadIdStack.o \
	$(OBJDIR)/stack.o \
	$(OBJDIR)/trie.o
# c,h folders
vpath %.c src
vpath %.h include

# all needs swordx
all: swordx

# swordx needs objects
swordx: $(OBJ)
	$(CC) $^ -pthread -Iinclude -o $@

# objects are into OBJDIR and needs .c and .h
$(OBJDIR)/%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

# swordx needs .c and all .h files
$(OBJDIR)/swordx.o: swordx.c ThreadIdStack.h trie.h stack.h BST.h
	$(CC) $(CFLAGS) -o $@ -c $<

# The .PHONY rule keeps make from doing something with a file named clean
.PHONY: clean
clean:
	rm swordx obj/*.o
