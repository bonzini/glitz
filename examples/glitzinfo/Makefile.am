INCLUDES = $(GLITZ_INC)

noinst_PROGRAMS =

if GLITZ_BUILD_AGL_BACKEND
noinst_PROGRAMS += glitzinfo-agl

glitzinfo_agl_CFLAGS = $(GLITZ_AGL_CFLAGS) -DGLITZ_AGL_BACKEND

glitzinfo_agl_SOURCES = glitzinfo.c

glitzinfo_agl_LDADD = $(GLITZ_LIB) $(GLITZ_AGL_LIBS)
endif

if GLITZ_BUILD_EGL_BACKEND
noinst_PROGRAMS += glitzinfo-egl

glitzinfo_egl_CFLAGS = $(GLITZ_EGL_CFLAGS)  -DGLITZ_EGL_BACKEND

glitzinfo_egl_SOURCES = glitzinfo.c

glitzinfo_egl_LDADD = $(GLITZ_LIB) $(GLITZ_EGL_LIBS)
endif

if GLITZ_BUILD_GLX_BACKEND
noinst_PROGRAMS += glitzinfo-glx

glitzinfo_glx_CFLAGS = $(GLITZ_GLX_CFLAGS) -DGLITZ_GLX_BACKEND

glitzinfo_glx_SOURCES = glitzinfo.c

glitzinfo_glx_LDADD = $(GLITZ_LIB) $(GLITZ_GLX_LIBS) 
endif
