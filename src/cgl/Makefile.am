if GLITZ_BUILD_CGL_BACKEND

INCLUDES =	      \
	$(GLITZ_INC)  \
	$(CGL_CFLAGS)

lib_LTLIBRARIES = libglitz-cgl.la
include_HEADERS = glitz-cgl.h

libglitz_cgl_la_SOURCES =     \
	glitz-cgl.h	      \
	glitz_cgl_drawable.m  \
	glitz_cgl_format.m    \
	glitz_cgl_info.m      \
	glitz_cgl_extension.m \
	glitz_cgl_context.m   \
	glitz_cgl_pbuffer.m   \
	glitz_cglint.h

libglitz_cgl_la_LDFLAGS = -version-info @VERSION_INFO@
libglitz_cgl_la_LIBADD = $(GLITZ_LIB) $(CGL_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = glitz-cgl.pc

endif

EXTRA_DIST =		      \
	glitz-cgl.h	      \
	glitz_cgl_drawable.c  \
	glitz_cgl_format.c    \
	glitz_cgl_info.c      \
	glitz_cgl_extension.c \
	glitz_cgl_context.c   \
	glitz_cgl_pbuffer.c   \
	glitz_cglint.h	      \
	glitz-cgl.pc.in	      \
	glitz-cgl.man
