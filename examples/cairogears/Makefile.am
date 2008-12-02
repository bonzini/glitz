INCLUDES = $(GLITZ_INC)

noinst_PROGRAMS =

CAIROGEARS_SRC = trap.c comp.c text.c shadow.c \
	fdface.c fdhand.c fdlogo.c common.c \
	gears.c opengl.c

cairogears: $(OBJS)
	gcc -o $@ $(OBJS) $(LDFLAGS)

all: $(PROGS)

clean:
	rm -f $(PROGS) *.o


if GLITZ_BUILD_AGL_BACKEND
noinst_PROGRAMS += cairogears-agl

cairogears_agl_CFLAGS = $(GLITZ_AGL_CFLAGS) $(CAIRO_CFLAGS)

cairogears_agl_SOURCES = cairogears-agl.c $(CAIROGEARS_SRC)

cairogears_agl_LDADD = $(GLITZ_LIB) $(GLITZ_AGL_LIBS) $(CARBON_LIBS) $(CAIRO_LIBS)
endif

if GLITZ_BUILD_GLX_BACKEND
noinst_PROGRAMS += cairogears-glx

cairogears_glx_CFLAGS = $(GLITZ_GLX_CFLAGS) $(CAIRO_CFLAGS)

cairogears_glx_SOURCES = cairogears-glx.c $(CAIROGEARS_SRC)

cairogears_glx_LDADD = $(GLITZ_LIB) $(GLITZ_GLX_LIBS)  $(CAIRO_LIBS)
endif
