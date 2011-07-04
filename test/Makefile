PERL	:= perl
CC		:= gcc
LD		:= gcc
OBJDIR 	:= obj
V		:= @

OBJDIRS	= 

objs	:= socketpair.o

CCFLAGS := -Wall -MD -O
LDFLAGS := -lrt -lpthread -lc 

.PHONY: all clean shmem_bin socketpair_bin


all: 

shmem: $(OBJDIR)/shmem_bin

ask: $(OBJDIR)/ask_bin

socketpair: $(OBJDIR)/socketpair_bin

# Include Makefrags for subdirectories
include socketpair/Makefrag
include shmem/Makefrag
include ask/Makefrag

clean:
	$(V)echo + rm $(OBJDIR)
	$(V)rm -rf $(OBJDIR)

# This magic automatically generates makefile dependencies
# for header files included from C source files we compile,
# and keeps those dependencies up-to-date every time we recompile.
# See 'mergedep.pl' for more information.
$(OBJDIR)/.deps: $(foreach dir, $(OBJDIRS), $(wildcard $(OBJDIR)/$(dir)/*.d))
	@mkdir -p $(@D)
	@$(PERL) mergedep.pl $@ $^

-include $(OBJDIR)/.deps

	
