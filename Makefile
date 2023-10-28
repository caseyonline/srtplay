CC=g++
CFLAGS=-I.
OBJ = srtplay.o \
			incommingMediaStream.o \
			incommingRemoteMediaStream.o \
			outgoingRemoteMediaStream.o

SYS=_$(shell uname -s)

LIBS += \
	-lpthread \
	-lcrypto

LIBSRTDIR = ./srt
include srt/Makefile.inc

srtplay: $(OBJ) libsrt$(SYS).a
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

libsrt$(SYS).a: $(LIBSRT_OBJS)
	$(AR) crv $@ $^

%.o: %.cxx
	$(CC) -c -o $@ $< $(CFLAGS)
