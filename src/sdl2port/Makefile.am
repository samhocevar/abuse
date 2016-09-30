
DEFS = @DEFS@

noinst_LIBRARIES = libsdlport.a

libsdlport_a_SOURCES = \
    video.cpp \
    event.cpp \
    sound.cpp sound.h \
    timing.cpp \
    jdir.cpp \
    joystick.cpp joy.h \
    setup.cpp setup.h \
    hmi.cpp hmi.h \
    $(NULL)
libsdlport_a_LIBADD =

CPPFLAGS = @CPPFLAGS@ -DASSETDIR=\"$(assetdir)\" \
           -I$(srcdir)/../ -I$(srcdir)/../imlib $(SDL_CFLAGS)

