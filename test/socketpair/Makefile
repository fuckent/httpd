CC		= gcc
OBJ 	= obj/
V		= @

objs	= socketpair.o

CCFLAG = -Wall -ggdb

.PHONY: all clean 


all: objdir $(OBJ)socketpair
	$(V)echo Finished!

objdir:
	

$(OBJ)socketpair: skpair.c 
	$(V)echo CC $@
	$(V)$(CC) $(CCFLAG) $^ -o $@

clean:
	$(V)echo RM obj/*
	$(V)rm -rf obj/*

