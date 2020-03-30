.PHONY: all, clean

# Disable implicit rules
.SUFFIXES:

# Keep intermediate files
#.PRECIOUS: %.o

CC = gcc
CFLAGS = -Wall -Werror
LDFLAGS =

# Note: -lnsl does not seem to work on Mac OS but will
# probably be necessary on Solaris for linking network-related functions 
#LIBS += -lsocket -lnsl -lrt
LIBS += -lpthread -lcrypt

INCLUDE = libs/csapp.h
OBJS = libs/csapp.o libs/utils.o server/backend.o 
INCLDIR = -I.

PROGS = server/server client/client


all: $(PROGS)

%.o: %.c $(INCLUDE)
	$(CC) $(CFLAGS) $(INCLDIR) -c -o $@ $<
	
%: %.o $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)
#	$(CC) -o $@ $(LDFLAGS) $(LIBS) $^
	
clean:
	rm -f libs/$(PROGS) client/client server/server server/*.o client/*.o libs/*.o client/downloads/* client/crash.log
