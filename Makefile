# Makefile

CLANG_FORMAT = clang-format-3.5

# TODO
include porting/Local.mk

clean:
	rm -f *.o

format:
	$(CLANG_FORMAT) -i *.cpp *.h bits/*.h import/*.h porting/*.cpp

ci:
	git clone https://github.com/godotengine/godot
	ln -s `pwd` godot/platform/frt
	( cd godot ; scons platform=frt target=release tools=no frt_arch=none )
