# Makefile

VPATH = . ..

CXXFLAGS = -I.. -DFRT_TEST -std=c++98 -Wall -ggdb
LDFLAGS = -ggdb
OBJS =
LIBS =

# TODO
include ../Local.mk

test.bin: $(OBJS)
	$(CXX) $(LDFLAGS) -o test.bin $(OBJS) $(LIBS)

clean:
	rm -f a.out *.bin *.o
