CC				=  gcc
AR 			= ar #
CFLAGS	  += -std=gnu99 -Wall -Werror -g
ARFLAGS 		= rvs
INCDIR      = ./headers
INCLUDES		= -I. -I $(INCDIR)
LDFLAGS 		= -L.
LIBS        = -lpthread
TEST        = test.sh

TARGETS     = farm generafile

.PHONY: all clean cleanall
.SUFFIXES: .c .h

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $< 


all  : $(TARGETS)

farm: main.o collector.a worker.a list.a master.a threadpool.a utils.a
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^ $(LIBS)


collector.a: collector.o headers/collector.h
	$(AR) $(ARFLAGS) $@ $<
master.a: master.o headers/master.h
	$(AR) $(ARFLAGS) $@ $<
worker.a: worker.o headers/worker.h
	$(AR) $(ARFLAGS) $@ $<
list.a: list.o headers/list.h
	$(AR) $(ARFLAGS) $@ $<
threadpool.a: threadpool.o headers/threadpool.h
	$(AR) $(ARFLAGS) $@ $<
utils.a: utils.o headers/utils.h
	$(AR) $(ARFLAGS) $@ $<

generafile: generafile.c
	$(CC) $(CFLAGS) -o $@ $^

test: farm generafile
	chmod +x $(TEST)
	./$(TEST)

clean		: 
	rm -f $(TARGETS)
cleanall	: clean
	\rm -f *.o *~ *.a *.dat
	-rm -r testdir
	-rm expected.txt
	-rm -f farm.sck
	-rm -f farm.s
	
