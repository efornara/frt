# Setup:
#   $ make local
#   $ ln -sf X11.mk Local.mk

# Test:
#   $ make run

# Done:
#   $ make restore
 
OBJS += \
	test_all.o \
	video_x11.o \
	keyboard_x11.o \
	mouse_x11.o \
	frt_options.o

ifdef HAS_GODOT
OBJS += key_mapping_x11_2.o
CXXFLAGS += -I../../.. -I../../../core
else
CXXFLAGS += -DFRT_MOCK_KEY_MAPPING_X11
endif

LIBS += -lEGL -lGLESv2 -lX11
