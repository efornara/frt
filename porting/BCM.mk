# Setup:
#   $ make local
#   $ ln -sf BCM.mk Local.mk

# Test:
#   $ make run

# Done:
#   $ make restore
 
OBJS += test_video.o video_bcm.o
CXXFLAGS += -I/opt/vc/include
LIBS += -L/opt/vc/lib -lbrcmGLESv2 -lbrcmEGL -lbcm_host
