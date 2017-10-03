# Makefile

CLANG_FORMAT = clang-format-3.5

include Local.mk

clean:
	rm -f *.o dl/*.gen.* import/*.o

format:
	$(CLANG_FORMAT) -i *.cpp *.h bits/*.h import/*.h porting/*.cpp

ci:
	git clone https://github.com/godotengine/godot
	ln -s `pwd` godot/platform/frt
	( cd godot ; scons platform=frt target=release warnings=no frt_arch=pc )
