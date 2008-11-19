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
#  include "../../config.h"
#endif

#include "glitz_cglint.h"

NSOpenGLPixelBuffer *
glitz_cgl_pbuffer_create (glitz_cgl_thread_info_t *thread_info,
			  int                     width,
			  int                     height)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSOpenGLPixelBuffer *pbuffer;
    glitz_gl_enum_t target;

    if (!POWER_OF_TWO (width) || !POWER_OF_TWO (height)) {
	if (thread_info->cgl_feature_mask &
	    GLITZ_CGL_FEATURE_TEXTURE_RECTANGLE_MASK)
	    target = GLITZ_GL_TEXTURE_RECTANGLE;
	else
	    return NULL;
    } else
	target = GLITZ_GL_TEXTURE_2D;

    pbuffer = [[NSOpenGLPixelBuffer alloc] initWithTextureTarget: target
	textureInternalFormat: GLITZ_GL_RGBA
	textureMaxMipMapLevel: 0 pixelsWide: width pixelsHigh: height];
    [pool release];
    return pbuffer;
}

void
glitz_cgl_pbuffer_destroy (NSOpenGLPixelBuffer *pbuffer)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [pbuffer release];
    [pool release];
}
