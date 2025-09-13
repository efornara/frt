FROM fedora:39

WORKDIR /root

RUN dnf -y install --setopt=install_weak_deps=False \
	bash bzip2 curl file findutils gettext git make nano patch pkgconfig python3-pip unzip which xz yasm && \
	pip install scons==4.5.2

ENV BASE_PATH=${PATH}

RUN cd /root && \
	curl -LO https://github.com/godotengine/buildroot/releases/download/godot-2023.08.x-4/aarch64-godot-linux-gnu_sdk-buildroot.tar.bz2 && \
	tar xf aarch64-godot-linux-gnu_sdk-buildroot.tar.bz2 && \
	rm -f aarch64-godot-linux-gnu_sdk-buildroot.tar.bz2 && \
	cd aarch64-godot-linux-gnu_sdk-buildroot && \
	./relocate-sdk.sh

RUN cd /root && \
	curl -LO https://github.com/godotengine/buildroot/releases/download/godot-2023.08.x-4/arm-godot-linux-gnueabihf_sdk-buildroot.tar.bz2 && \
	tar xf arm-godot-linux-gnueabihf_sdk-buildroot.tar.bz2 && \
	rm -f arm-godot-linux-gnueabihf_sdk-buildroot.tar.bz2 && \
	cd arm-godot-linux-gnueabihf_sdk-buildroot && \
	./relocate-sdk.sh

ENV GODOT_SDK_LINUX_ARM64=/root/aarch64-godot-linux-gnu_sdk-buildroot
ENV GODOT_SDK_LINUX_ARM32=/root/arm-godot-linux-gnueabihf_sdk-buildroot

RUN cd /root && \
	SDL2VER=2.32.10 && \
	curl -LO https://github.com/libsdl-org/SDL/releases/download/release-${SDL2VER}/SDL2-${SDL2VER}.tar.gz && \
	tar xf SDL2-${SDL2VER}.tar.gz && \
	rm -f SDL2-${SDL2VER}.tar.gz && \
	mkdir obj && cd obj && \
	PATH=${GODOT_SDK_LINUX_ARM64}/bin:${BASE_PATH} && \
	../SDL2-${SDL2VER}/configure --prefix=/root/sdl2/arm64 --host=aarch64-linux-gnu && \
	make && make install && cd .. && rm -r obj && \
	mkdir obj && cd obj && \
	PATH=${GODOT_SDK_LINUX_ARM32}/bin:${BASE_PATH} && \
	../SDL2-${SDL2VER}/configure --prefix=/root/sdl2/arm32 --host=arm-linux-gnueabihf && \
	make && make install && cd .. && rm -r obj && \
	rm -r SDL2-${SDL2VER}

ENV SDL2_ARM64=/root/sdl2/arm64
ENV SDL2_ARM32=/root/sdl2/arm32

RUN chmod 755 /root
