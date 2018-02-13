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
	( cd godot ; scons platform=frt target=release warnings=no builtin_freetype=yes builtin_libpng=yes builtin_zlib=yes module_openssl_enabled=no module_websocket_enabled=no tools=no frt_arch=pc )
