INCLUDES = -I$(top_builddir) -I$(top_srcdir)
LDADD = $(top_builddir)/fob/libfob-1.0.la -lpthread

EXTRA_DIST = README

noinst_PROGRAMS = \
	fly \
	fly2 

fly_SOURCES = \
	args.h \
	commandoptions.cpp commandoptions.h \
	fly.cpp

fly2_LDADD = \
	$(LDADD) \
	-lGL -lGLU -lglut
fly2_SOURCES = \
	args.h \
	commandoptions.cpp commandoptions.h \
	fly2.cpp \
	camera.cpp camera.h
