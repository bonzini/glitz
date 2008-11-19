/*
 * Copyright © 2004 David Reveman
 * Copyright © 2008 Paolo Bonzini
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of the
 * copyright holders not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * The copyright holders make no representations about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID REVEMAN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 * Author: Paolo Bonzini <bonzini@gnu.org>
 */

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitz_cglint.h"

static glitz_extension_map cgl_extensions[] = {
    { 0.0, "GL_APPLE_pixel_buffer", GLITZ_CGL_FEATURE_PBUFFER_MASK },
    { 0.0, "GL_ARB_multisample", GLITZ_CGL_FEATURE_MULTISAMPLE_MASK },
    { 0.0, "GL_ARB_texture_rectangle",
      GLITZ_CGL_FEATURE_TEXTURE_RECTANGLE_MASK },
    { 0.0, "GL_EXT_texture_rectangle",
      GLITZ_CGL_FEATURE_TEXTURE_RECTANGLE_MASK },
    { 0.0, "GL_NV_texture_rectangle",
      GLITZ_CGL_FEATURE_TEXTURE_RECTANGLE_MASK },
    { 0.0, NULL, 0 }
};

static int
glitz_test_pbuffer_format (CGLPixelFormatAttribute *attrib)
{
    CGLContextObj ctx = NULL;
    CGLPixelFormatObj pix = NULL;
    CGLPBufferObj pb = NULL;
    GLint npix;
    long screen;
    int result;

    result =
       (CGLChoosePixelFormat (attrib, &pix, &npix) == kCGLNoError
	&& npix > 0
	&& CGLCreatePBuffer (2, 2, GLITZ_GL_TEXTURE_2D, GL_RGBA, 0, &pb) == kCGLNoError
	&& CGLCreateContext (pix, NULL, &ctx) == kCGLNoError
	&& CGLGetVirtualScreen (ctx, &screen) == kCGLNoError
	&& CGLSetPBuffer (ctx, pb, 0, 0, screen) == kCGLNoError);

    if (ctx != NULL)
	CGLDestroyContext (ctx);
    if (pb != NULL)
	CGLDestroyPBuffer (pb);
    if (pix != NULL)
	CGLDestroyPixelFormat (pix);
    return result;
}

static CGLPixelFormatAttribute _default_attrib[] = {
    kCGLPFANoRecovery,
    0
};

static CGLPixelFormatAttribute _db_attrib[] = {
    kCGLPFADoubleBuffer,
    kCGLPFANoRecovery,
    0
};

static CGLPixelFormatAttribute _depth_attrib[] = {
    kCGLPFADepthSize, 8,
    kCGLPFAStencilSize, 8,
    kCGLPFANoRecovery,
    0
};

static CGLPixelFormatAttribute _ms_attrib[] = {
    kCGLPFASampleBuffers, 1,
    kCGLPFASamples, 2,
    kCGLPFANoRecovery,
    0
};

glitz_status_t
glitz_cgl_query_extensions (glitz_cgl_thread_info_t *thread_info)
{
    const char *gl_extensions_string;
    CGLContextObj context;
    CGLPixelFormatObj pix;
    GLint npix;

    thread_info->cgl_feature_mask = 0;

    if (CGLChoosePixelFormat (_default_attrib, &pix, &npix) != kCGLNoError)
	return GLITZ_STATUS_SUCCESS;

    if (CGLCreateContext (pix, NULL, &context) != kCGLNoError)
	return GLITZ_STATUS_SUCCESS;

    CGLSetCurrentContext (context);
    gl_extensions_string = (const char *) glGetString (GL_EXTENSIONS);

    thread_info->cgl_feature_mask =
	glitz_extensions_query (0.0,
				gl_extensions_string,
				cgl_extensions);

    CGLSetCurrentContext (NULL);
    CGLDestroyContext (context);
    CGLDestroyPixelFormat (pix);

    if (glitz_test_pbuffer_format (_db_attrib))
    {
	thread_info->cgl_feature_mask |=
	   GLITZ_CGL_FEATURE_PBUFFER_DOUBLEBUFFER_MASK;
    }

    if (glitz_test_pbuffer_format (_depth_attrib))
    {
	thread_info->cgl_feature_mask |=
	   GLITZ_CGL_FEATURE_PBUFFER_DEPTH_STENCIL_MASK;
    }

    if ((thread_info->cgl_feature_mask & GLITZ_CGL_FEATURE_MULTISAMPLE_MASK)
	&& glitz_test_pbuffer_format (_ms_attrib))
    {
	thread_info->cgl_feature_mask |=
	   GLITZ_CGL_FEATURE_PBUFFER_MULTISAMPLE_MASK;
    }
    return GLITZ_STATUS_SUCCESS;
}
