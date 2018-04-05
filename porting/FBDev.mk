# Setup:
#   $ make local
#   $ ln -sf FBDev.mk Local.mk

# Test:
#   $ make run

# Done:
#   $ make restore
 
OBJS += \
	test_video.o \
	video_fbdev.o \
	frt_options.o

LIBS += -lEGL -lGLESv2
