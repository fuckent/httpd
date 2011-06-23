CC		= gcc
OBJ 	= obj/
V		= @

objs	= shmem.o

CCFLAG = -Wall -ggdb -lrt -lpthread

.PHONY: all clean 


all: objdir $(OBJ)shmem
	$(V)echo Finished!

objdir:
	

$(OBJ)shmem: shmem.c 
	$(V)echo CC $@
	$(V)$(CC) $(CCFLAG) $^ -o $@

clean:
	$(V)echo RM obj/*
	$(V)rm -rf obj/*

