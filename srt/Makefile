SYS ?= _$(shell uname -s)

-include Makefile.inc

libsrt$(SYS).a: $(LIBSRT_OBJS)
	$(AR) crv $@ $^

clean:
	$(RM) libsrt$(SYS).a $(LIBSRT_OBJS)

.SUFFIXES:	.cpp .c

%$(SYS).o: %.c
	$(CC) $(CCFLAGS) $(INC) -c -o $@ $<

%$(SYS).o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c -o $@ $<
