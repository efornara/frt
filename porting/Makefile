# Makefile

VPATH = . ..

CXXFLAGS = -I.. -DFRT_TEST -std=c++98 -Wall -ggdb
LDFLAGS = -ggdb
OBJS =
LIBS =

include Local.mk

test.bin: $(OBJS)
	$(CXX) $(LDFLAGS) -o test.bin $(OBJS) $(LIBS)

run: test.bin
	./test.bin

local:
	git update-index --assume-unchanged Local.mk

restore:
	git checkout Local.mk
	git update-index --no-assume-unchanged Local.mk

clean:
	rm -f a.out *.bin *.o
