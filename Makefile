# Makefile

CLANG_FORMAT = clang-format-3.5
CXXFLAGS = -DFRT_TEST -std=c++98 -Wall -ggdb
LDFLAGS = -ggdb
OBJS =
LIBS =

include Local.mk

test.bin: $(OBJS)
	$(CXX) $(LDFLAGS) -o test.bin $(OBJS) $(LIBS)

clean:
	rm -f *.bin *.o

format:
	$(CLANG_FORMAT) -i *.cpp *.h
