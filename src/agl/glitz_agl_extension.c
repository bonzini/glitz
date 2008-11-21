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

#include "glitz_aglint.h"

static glitz_extension_map agl_extensions[] = {
    { 0.0, "GL_APPLE_pixel_buffer", GLITZ_AGL_FEATURE_PBUFFER_MASK },
    { 0.0, "GL_ARB_multisample", GLITZ_AGL_FEATURE_MULTISAMPLE_MASK },
    { 0.0, "GL_ARB_texture_rectangle",
      GLITZ_AGL_FEATURE_TEXTURE_RECTANGLE_MASK },
    { 0.0, "GL_EXT_texture_rectangle",
      GLITZ_AGL_FEATURE_TEXTURE_RECTANGLE_MASK },
    { 0.0, "GL_NV_texture_rectangle",
      GLITZ_AGL_FEATURE_TEXTURE_RECTANGLE_MASK },
    { 0.0, NULL, 0 }
};

static int
glitz_test_pbuffer_format (glitz_agl_thread_info_t *thread_info,
			   GLint *attrib)
{
    AGLContext ctx = NULL;
    AGLPixelFormat pix = NULL;
    AGLPbuffer pb = NULL;
    int result;

    result =
       ((pix = aglChoosePixelFormat (NULL, 0, attrib)) != NULL
	&& (pb = glitz_agl_pbuffer_create (thread_info, 2, 2)) != NULL
	&& (ctx = aglCreateContext (pix, NULL)) != NULL
	&& aglSetPBuffer (ctx, pb, 0, 0,
			  aglGetVirtualScreen (ctx)) == AGL_NO_ERROR);

    if (ctx != NULL)
	aglDestroyContext (ctx);
    if (pb != NULL)
	aglDestroyPBuffer (pb);
    if (pix != NULL)
	aglDestroyPixelFormat (pix);
    return result;
}

static GLint _default_attrib[] = {
    AGL_RGBA,
    AGL_NO_RECOVERY,
    AGL_NONE
};

static GLint _db_attrib[] = {
    AGL_RGBA,
    AGL_DOUBLEBUFFER,
    AGL_NO_RECOVERY,
    AGL_NONE
};

static GLint _depth_attrib[] = {
    AGL_RGBA,
    AGL_NO_RECOVERY,
    AGL_DEPTH_SIZE, 8,
    AGL_STENCIL_SIZE, 8,
    AGL_NONE
};

static GLint _ms_attrib[] = {
    AGL_RGBA,
    AGL_NO_RECOVERY,
    AGL_SAMPLE_BUFFERS_ARB, 1,
    AGL_SAMPLES_ARB, 2,
    AGL_NONE
};

glitz_status_t
glitz_agl_query_extensions (glitz_agl_thread_info_t *thread_info)
{
    AGLContext context;
    AGLPixelFormat pf;

    thread_info->agl_feature_mask = 0;

    pf = aglChoosePixelFormat (NULL, 0, _default_attrib);
    context = aglCreateContext (pf, NULL);

    if (context) {
        const char *gl_extensions_string;

        aglSetCurrentContext (context);

        gl_extensions_string = (const char *) glGetString (GL_EXTENSIONS);

        thread_info->agl_feature_mask =
            glitz_extensions_query (0.0,
                                    gl_extensions_string,
                                    agl_extensions);

        aglSetCurrentContext (NULL);
        aglDestroyContext (context);

        if (glitz_test_pbuffer_format (thread_info, _db_attrib))
        {
	    thread_info->agl_feature_mask |=
	       GLITZ_AGL_FEATURE_PBUFFER_DOUBLEBUFFER_MASK;
        }

        if (glitz_test_pbuffer_format (thread_info, _depth_attrib))
        {
	    thread_info->agl_feature_mask |=
	       GLITZ_AGL_FEATURE_PBUFFER_DEPTH_STENCIL_MASK;
        }

        if ((thread_info->agl_feature_mask & GLITZ_AGL_FEATURE_MULTISAMPLE_MASK)
	    && glitz_test_pbuffer_format (thread_info, _ms_attrib))
        {
	    thread_info->agl_feature_mask |=
	       GLITZ_AGL_FEATURE_PBUFFER_MULTISAMPLE_MASK;
        }
    }

    aglDestroyPixelFormat (pf);

    return GLITZ_STATUS_SUCCESS;
}
