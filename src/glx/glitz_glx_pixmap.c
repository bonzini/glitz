/*
 * Copyright © 2004 David Reveman
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * David Reveman not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * David Reveman makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DAVID REVEMAN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID REVEMAN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 *	   Nicolas Bruguier <nicolas.bruguier@supersonicimagine.fr>
 */

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitz_glxint.h"

GLXPixmap
glitz_glx_pixmap_create (glitz_glx_screen_info_t *screen_info,
			 Pixmap 		 pixmap,
			 glitz_drawable_format_t *format,
			 unsigned int             width,
			 unsigned int             height)
{
    Display *dpy = screen_info->display_info->display;
    glitz_glx_context_t* ctx;
    GLXPixmap glx_pixmap;
    glitz_int_drawable_format_t* iformat = &screen_info->formats[format->id];
    
    if (!pixmap)
	return (GLXPixmap) 0;
    
    ctx = glitz_glx_context_get(screen_info, format);
    if (ctx)
    {
	unsigned int target = 0;
	int attribs[7];
	int cpt = 0;
	
	if ((iformat->texture_target & GLX_TEXTURE_2D_BIT_EXT) &&
	    ((ctx->backend.feature_mask & GLITZ_FEATURE_TEXTURE_NON_POWER_OF_TWO_MASK) ||
	     (POWER_OF_TWO (width) && POWER_OF_TWO (height))))
	    target = GLITZ_GL_TEXTURE_2D_EXT;
	else if (iformat->texture_target & GLX_TEXTURE_RECTANGLE_BIT_EXT)
	    target = GLITZ_GL_TEXTURE_RECTANGLE_EXT;
	
	if (!target)
	{
	    if (!(iformat->texture_target & GLX_TEXTURE_2D_BIT_EXT))
		target = GLITZ_GL_TEXTURE_RECTANGLE_EXT;
	    else if (!(iformat->texture_target & GLX_TEXTURE_RECTANGLE_BIT_EXT))
		target = GLITZ_GL_TEXTURE_2D_EXT;
	}
	
	if (target)
	{
	    attribs[cpt++] = GLITZ_GL_TEXTURE_TARGET_EXT;
	    attribs[cpt++] = target;
	}
	attribs[cpt++] = GLITZ_GL_TEXTURE_FORMAT_EXT;
	attribs[cpt++] = iformat->texture_format;
	attribs[cpt++] = GLITZ_GL_MIPMAP_TEXTURE_EXT,
	attribs[cpt++] = iformat->mipmap;
	
	attribs[cpt++] = 0;
	
	glx_pixmap = glXCreatePixmap (dpy, ctx->fbconfig, pixmap, attribs);
	
	return glx_pixmap;
    } else
	return (GLXPixmap) 0;
}

void
glitz_glx_pixmap_destroy (glitz_glx_screen_info_t *screen_info,
			  GLXPixmap                pixmap)
{
    Display *dpy = screen_info->display_info->display;
    
    glXDestroyGLXPixmap (dpy, pixmap);
}
