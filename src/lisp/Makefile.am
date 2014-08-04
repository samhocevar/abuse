## Process this file with automake to produce Makefile.in
noinst_LIBRARIES = liblisp.a

liblisp_a_SOURCES = \
    lisp.cpp lisp.h \
    lisp_opt.cpp lisp_opt.h \
    lisp_gc.cpp lisp_gc.h \
    trig.cpp \
    stack.h symbols.h \
    $(NULL)

CPPFLAGS = @CPPFLAGS@ -I$(srcdir) -I$(srcdir)/../ -I$(srcdir)/../imlib $(SDL_CFLAGS)

