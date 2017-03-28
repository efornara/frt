# Makefile

CLANG_FORMAT = clang-format-3.5

# TODO
include porting/Local.mk

clean:
	rm -f *.o

format:
	$(CLANG_FORMAT) -i *.cpp *.h bits/*.h import/*.h porting/*.cpp
