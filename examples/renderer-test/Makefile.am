if HAVE_LIBPNG

noinst_PROGRAMS =

rendertest_common_sources = \
	args.c \
	png.c \
	rendertest.c \
	rendertest.h \
	images.h \
	rects.h \
	traps.h

glitz_common_sources = \
	$(rendertest_common_sources) \
	glitz.c \
	glitz_common.h

INCLUDES = $(GLITZ_GLX_CFLAGS) $(GLITZ_EGL_CFLAGS) $(GLITZ_AGL_CFLAGS) \
           $(LIBPNG_CFLAGS)

if GLITZ_BUILD_GLX_BACKEND
noinst_PROGRAMS += rendertest_glitz_glx

rendertest_glitz_glx_SOURCES = \
	$(glitz_common_sources) \
	glitz_glx.c

rendertest_glitz_glx_LDADD = $(GLITZ_GLX_LIBS) $(LIBPNG_LIBS) -lm
endif

if GLITZ_BUILD_EGL_BACKEND
noinst_PROGRAMS += rendertest_glitz_egl

rendertest_glitz_egl_SOURCES = \
	$(glitz_common_sources) \
	glitz_egl.c

rendertest_glitz_egl_LDADD = $(GLITZ_EGL_LIBS) $(LIBPNG_LIBS) -lm
endif

if GLITZ_BUILD_AGL_BACKEND
noinst_PROGRAMS += rendertest_glitz_agl

rendertest_glitz_agl_SOURCES = \
	$(glitz_common_sources) \
	glitz_agl.c

rendertest_glitz_agl_LDADD = $(GLITZ_AGL_LIBS) $(CARBON_LIBS) $(LIBPNG_LIBS) -lm
endif

endif
