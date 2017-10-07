# Setup:
#   $ make local
#   $ ln -sf BCM.mk Local.mk

# Test:
#   $ make run

# Done:
#   $ make restore
 
OBJS += \
	test_video.o \
	video_bcm.o \
	frt_options.o \
	bcm.gen.o \
	egl.gen.o \
	gles2.gen.o

CXXFLAGS += -I/opt/vc/include
LIBS += -ldl
