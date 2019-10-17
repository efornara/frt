# Setup:
#   $ make local
#   $ ln -sf BCM.mk Local.mk

# Test:
#   $ make run

# Done:
#   $ make restore

OBJS += \
	test_video.o \
	video_kmsdrm.o \
	frt_options.o \
	egl.gen.o \
	gles2.gen.o

LIBS += -ldl -ldrm -lgbm -lEGL -lGLESv2
