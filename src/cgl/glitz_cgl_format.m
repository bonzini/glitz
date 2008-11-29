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

#include <stdlib.h>
#include <string.h>

struct pixel_format_info {
    int doublebuffer, depth_size, stencil_size, samples;
};

static struct pixel_format_info _attribs_list[] = {
    /* Explicitly request no depth/no stencil to discover pbuffer formats.
       Apparently things are a little different for AGL and CGL here.  */
    { 0, 0, 0, 1 },
    { 1, 0, 0, 1 },

    /* With stencil (why also NSOpenGLPFADepthSize for no double buffering?
       I took this from glitz_agl_format.c).  */
    { 0, 8, 8, 1 },
    { 1, -1, 8, 1 },

    /* Multi-sampled formats.  */
    { 0, -1, 8, 2 },
    { 1, -1, 8, 2 },

    { 0, -1, 8, 4 },
    { 1, -1, 8, 4 },

    { 0, -1, 8, 6 },
    { 1, -1, 8, 6 }
};

static int
_glitz_cgl_format_compare (const void *elem1,
			   const void *elem2)
{
    glitz_int_drawable_format_t *format[2];
    int				i, score[2];

    format[0] = (glitz_int_drawable_format_t *) elem1;
    format[1] = (glitz_int_drawable_format_t *) elem2;
    i = score[0] = score[1] = 0;

    for (; i < 2; i++)
    {
	if (format[i]->d.color.red_size)
	{
	    if (format[i]->d.color.red_size >= 8)
		score[i] += 5;

	    score[i] += 10;
	}

	if (format[i]->d.color.alpha_size)
	{
	    if (format[i]->d.color.alpha_size >= 8)
		score[i] += 5;

	    score[i] += 10;
	}

	if (format[i]->d.stencil_size)
	    score[i] += 3;

	if (format[i]->d.depth_size)
	    score[i] += 3;

	if (format[i]->d.doublebuffer)
	    score[i] += 10;

	if (format[i]->d.samples > 1)
	    score[i] -= (20 - format[i]->d.samples / 2);

	if (format[i]->types & GLITZ_DRAWABLE_TYPE_WINDOW_MASK)
	    score[i] += 10;

	if (format[i]->types & GLITZ_DRAWABLE_TYPE_PBUFFER_MASK)
	    score[i] += 10;

	if (format[i]->caveat)
	    score[i] -= 1000;
    }

    return score[1] - score[0];
}

static void
_glitz_cgl_add_format (glitz_cgl_thread_info_t     *thread_info,
		       glitz_int_drawable_format_t *format,
		       NSOpenGLPixelFormat         *pixel_format)
{
    if (!glitz_drawable_format_find (thread_info->formats,
				     thread_info->n_formats,
				     GLITZ_DRAWABLE_FORMAT_ALL_EXCEPT_ID_MASK,
				     format, 0)) {
	int n = thread_info->n_formats;

	thread_info->formats =
	    realloc (thread_info->formats,
		     sizeof (glitz_int_drawable_format_t) * (n + 1));
	thread_info->pixel_formats =
	    realloc (thread_info->pixel_formats,
		     sizeof (NSOpenGLPixelFormat *) * (n + 1));

	if (thread_info->formats && thread_info->pixel_formats) {
	    thread_info->formats[n] = *(glitz_int_drawable_format_t*)format;
	    thread_info->formats[n].d.id = n;
	    thread_info->pixel_formats[n] = pixel_format;
	    thread_info->n_formats++;
	}
    }
}

static GLint
glitz_cgl_describe_format (NSOpenGLPixelFormat *pixel_format, int attrib)
{
    GLint value;
    [pixel_format getValues: &value forAttribute: attrib forVirtualScreen: 0];
    return value;
}

void
glitz_cgl_query_formats (glitz_cgl_thread_info_t *thread_info)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    glitz_int_drawable_format_t format;
    NSOpenGLPixelFormat *pixel_format, **new_pfs;
    int n_attribs_list, i;

    NSScreen * screenWithMenubar = [[NSScreen screens] objectAtIndex: 0];
    int color_size = NSBitsPerPixelFromDepth([screenWithMenubar depth]);
    NSOpenGLPixelFormatAttribute attribs[16] = {
	NSOpenGLPFAAlphaSize, (color_size <= 16) ? 1 : 8,
	NSOpenGLPFAColorSize, (color_size <= 16) ? 16 : 32 };

    format.types	    = GLITZ_DRAWABLE_TYPE_WINDOW_MASK;
    format.d.id		    = 0;
    format.d.color.fourcc   = GLITZ_FOURCC_RGB;
    format.d.scanline_order = GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN;
    
    n_attribs_list = sizeof (_attribs_list) / sizeof (GLint *);

    for (i = 0; i < n_attribs_list; i++) {
	GLint value;
	CGLPixelFormatObj pix;
	int j = 4;

	if (_attribs_list[i].doublebuffer)
	  attribs[j++] = NSOpenGLPFADoubleBuffer;
	if (_attribs_list[i].depth_size > -1)
	  {
	    attribs[j++] = NSOpenGLPFADepthSize;
	    attribs[j++] = _attribs_list[i].depth_size;
	  }
	if (_attribs_list[i].stencil_size > -1)
	  {
	    attribs[j++] = NSOpenGLPFAStencilSize;
	    attribs[j++] = _attribs_list[i].stencil_size;
	  }
	if (_attribs_list[i].samples > 1)
	  {
	    attribs[j++] = NSOpenGLPFASampleBuffers;
	    attribs[j++] = 1;
	    attribs[j++] = NSOpenGLPFASamples;
	    attribs[j++] = _attribs_list[i].samples;
	  }
	attribs[j++] = 0;

	/* NSOpenGLFormat logs an error message if given an invalid format, so try
	   the CGL function first.  */
	if (CGLChoosePixelFormat ((CGLPixelFormatAttribute *) attribs,
				  &pix, &value) != kCGLNoError)
	    continue;

	CGLDestroyPixelFormat (pix);
        pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];

	/* Stereo is not supported yet */
	if (glitz_cgl_describe_format (pixel_format, NSOpenGLPFAStereo)) {
	    [pixel_format release];
	    continue;
	}

	value = glitz_cgl_describe_format (pixel_format, NSOpenGLPFAAccelerated);
	format.caveat = (value)? 0: 1;
	value = glitz_cgl_describe_format (pixel_format, NSOpenGLPFADoubleBuffer);
	format.d.doublebuffer = (value)? 1: 0;

	value = glitz_cgl_describe_format (pixel_format, NSOpenGLPFAAlphaSize);
	format.d.color.alpha_size = (unsigned short) value;
	value = glitz_cgl_describe_format (pixel_format, NSOpenGLPFAColorSize) - value;
	format.d.color.red_size = (unsigned short) (value + 1) / 3;
	format.d.color.green_size = (unsigned short) (value + 2) / 3;
	format.d.color.blue_size = (unsigned short) value / 3;
	value = glitz_cgl_describe_format (pixel_format, NSOpenGLPFADepthSize);
	format.d.depth_size = (unsigned short) value;
	value = glitz_cgl_describe_format (pixel_format, NSOpenGLPFAStencilSize);
	format.d.stencil_size = (unsigned short) value;

	if (thread_info->cgl_feature_mask & GLITZ_CGL_FEATURE_MULTISAMPLE_MASK)
	{
	    value = glitz_cgl_describe_format (pixel_format, NSOpenGLPFASampleBuffers);
	    if (value) {
		value = glitz_cgl_describe_format (pixel_format, NSOpenGLPFASamples);
		format.d.samples = (unsigned short) (value > 1)? value: 1;
	    } else
		format.d.samples = 1;
	} else
	    format.d.samples = 1;

	format.types |= GLITZ_DRAWABLE_TYPE_PBUFFER_MASK;
	if ((thread_info->cgl_feature_mask &
	     GLITZ_CGL_FEATURE_PBUFFER_MASK) == 0)
	    format.types &= ~GLITZ_DRAWABLE_TYPE_PBUFFER_MASK;

	else if (format.d.color.red_size == 0 ||
		 format.d.color.green_size == 0 ||
		 format.d.color.blue_size == 0 ||
		 format.d.color.alpha_size == 0)
	    format.types &= ~GLITZ_DRAWABLE_TYPE_PBUFFER_MASK;

	else if ((thread_info->cgl_feature_mask &
		  GLITZ_CGL_FEATURE_PBUFFER_DOUBLEBUFFER_MASK) == 0
		 && format.d.doublebuffer)
	    format.types &= ~GLITZ_DRAWABLE_TYPE_PBUFFER_MASK;

	else if ((thread_info->cgl_feature_mask &
		  GLITZ_CGL_FEATURE_PBUFFER_MULTISAMPLE_MASK) == 0
		 && format.d.samples > 1)
	    format.types &= ~GLITZ_DRAWABLE_TYPE_PBUFFER_MASK;

	else if ((thread_info->cgl_feature_mask &
		  GLITZ_CGL_FEATURE_PBUFFER_DEPTH_STENCIL_MASK) == 0
		 && (format.d.depth_size > 0 || format.d.stencil_size > 0))
	    format.types &= ~GLITZ_DRAWABLE_TYPE_PBUFFER_MASK;

	if (format.d.color.red_size ||
	    format.d.color.green_size ||
	    format.d.color.blue_size ||
	    format.d.color.alpha_size)
	    _glitz_cgl_add_format (thread_info, &format, pixel_format);
    }

    if (!thread_info->n_formats)
	return;

    qsort (thread_info->formats, thread_info->n_formats,
	   sizeof (glitz_int_drawable_format_t), _glitz_cgl_format_compare);

    /*
     * Update NSOpenGLPixelFormat list so that it matches the sorted format list.
     */
    new_pfs = malloc (sizeof (NSOpenGLPixelFormat *) * thread_info->n_formats);
    if (!new_pfs) {
	thread_info->n_formats = 0;
	return;
    }

    for (i = 0; i < thread_info->n_formats; i++) {
	new_pfs[i] = thread_info->pixel_formats[thread_info->formats[i].d.id];
	thread_info->formats[i].d.id = i;
    }

    free (thread_info->pixel_formats);
    thread_info->pixel_formats = new_pfs;
    [pool release];
}

glitz_drawable_format_t *
glitz_cgl_find_window_format (unsigned long                 mask,
			      const glitz_drawable_format_t *templ,
			      int                           count)
{
    glitz_int_drawable_format_t itempl;
    glitz_cgl_thread_info_t *thread_info =
	glitz_cgl_thread_info_get ();

    glitz_drawable_format_copy (templ, &itempl.d, mask);

    itempl.types = GLITZ_DRAWABLE_TYPE_WINDOW_MASK;
    mask |= GLITZ_INT_FORMAT_WINDOW_MASK;

    return glitz_drawable_format_find (thread_info->formats,
				       thread_info->n_formats,
				       mask, &itempl, count);
}
slim_hidden_def(glitz_cgl_find_window_format);


glitz_drawable_format_t *
glitz_cgl_find_pbuffer_format (unsigned long                 mask,
			      const glitz_drawable_format_t *templ,
			      int                           count)
{
    glitz_int_drawable_format_t itempl;
    glitz_cgl_thread_info_t *thread_info =
	glitz_cgl_thread_info_get ();

    glitz_drawable_format_copy (templ, &itempl.d, mask);

    itempl.types = GLITZ_DRAWABLE_TYPE_PBUFFER_MASK;
    mask |= GLITZ_INT_FORMAT_PBUFFER_MASK;

    return glitz_drawable_format_find (thread_info->formats,
				       thread_info->n_formats,
				       mask, &itempl, count);
}
slim_hidden_def(glitz_cgl_find_pbuffer_format);

