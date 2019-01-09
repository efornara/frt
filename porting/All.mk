# Setup:
#   $ make local
#   $ ln -sf All.mk Local.mk

# Test:
#   $ make run

# Done:
#   $ make restore
 
OBJS += \
	test_all.o \
	video_x11.o \
	keyboard_x11.o \
	mouse_x11.o \
	video_fbdev.o \
	keyboard_linux_input.o \
	mouse_linux_input.o \
	envprobe.o \
	frt_options.o \
	x11.gen.o \
	egl.gen.o \
	gles2.gen.o

ifdef HAS_GODOT
OBJS += key_mapping_x11_2.o
CXXFLAGS += -I../../..
else
CXXFLAGS += -DFRT_MOCK_KEY_MAPPING_X11
endif

ifdef HAS_BCM
OBJS += video_bcm.o bcm.gen.o
CXXFLAGS += -I/opt/vc/include
endif

LIBS += -ldl
