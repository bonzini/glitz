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

static glitz_cgl_drawable_t *
_glitz_cgl_create_drawable (glitz_cgl_thread_info_t *thread_info,
			    glitz_cgl_context_t     *context,
			    glitz_drawable_format_t *format,
			    NSView                  *view,
			    NSOpenGLPixelBuffer     *pbuffer,
			    unsigned int            width,
			    unsigned int            height)
{
    glitz_cgl_drawable_t *drawable;

    drawable = (glitz_cgl_drawable_t *) malloc (sizeof (glitz_cgl_drawable_t));
    if (drawable == NULL)
	return NULL;

    drawable->thread_info = thread_info;
    drawable->context = context;
    drawable->view = view;
    drawable->pbuffer = pbuffer;
    drawable->width = width;
    drawable->height = height;

    _glitz_drawable_init (&drawable->base,
			  &thread_info->formats[format->id],
			  &context->backend,
			  width, height);

    if (!context->initialized) {
	glitz_cgl_push_current (drawable, NULL, GLITZ_CONTEXT_CURRENT, NULL);
	glitz_cgl_pop_current (drawable);
    }

    if (width > context->backend.max_viewport_dims[0] ||
	height > context->backend.max_viewport_dims[1]) {
	free (drawable);
	return NULL;
    }

    thread_info->drawables++;

    return drawable;
}

glitz_bool_t
glitz_cgl_drawable_update_size (glitz_cgl_drawable_t *drawable,
				int                  width,
				int                  height)
{
    if (drawable->pbuffer)
    {
	glitz_cgl_pbuffer_destroy (drawable->pbuffer);
	drawable->pbuffer =
	    glitz_cgl_pbuffer_create (drawable->thread_info,
				      (int) width, (int) height);
	if (!drawable->pbuffer)
	    return 0;
    }

    drawable->width  = width;
    drawable->height = height;

    return 1;
}

static glitz_drawable_t *
_glitz_cgl_create_pbuffer_drawable (glitz_cgl_thread_info_t *thread_info,
				    glitz_drawable_format_t *format,
				    unsigned int            width,
				    unsigned int            height)
{
    glitz_cgl_drawable_t *drawable;
    glitz_cgl_context_t *context;
    NSOpenGLPixelBuffer *pbuffer;

    context = glitz_cgl_context_get (thread_info, format);
    if (!context)
	return NULL;

    pbuffer = glitz_cgl_pbuffer_create (thread_info,
					(int) width, (int) height);
    if (!pbuffer)
	return NULL;

    drawable = _glitz_cgl_create_drawable (thread_info, context, format,
					   NULL, pbuffer,
					   width, height);
    if (!drawable) {
	glitz_cgl_pbuffer_destroy (pbuffer);
	return NULL;
    }

    return &drawable->base;
}

glitz_drawable_t *
glitz_cgl_create_pbuffer (void                    *abstract_templ,
			  glitz_drawable_format_t *format,
			  unsigned int            width,
			  unsigned int            height)
{
    glitz_cgl_drawable_t *templ = (glitz_cgl_drawable_t *) abstract_templ;

    return _glitz_cgl_create_pbuffer_drawable (templ->thread_info, format,
					       width, height);
}

glitz_drawable_t *
glitz_cgl_create_drawable_for_view (glitz_drawable_format_t *format,
				    NSView                  *view,
				    unsigned int            width,
				    unsigned int            height)
{
    glitz_cgl_drawable_t *drawable;
    glitz_cgl_thread_info_t *thread_info;
    glitz_cgl_context_t *context;

    thread_info = glitz_cgl_thread_info_get ();
    if (!thread_info)
	return NULL;

    if (format->id >= thread_info->n_formats)
	return NULL;

    context = glitz_cgl_context_get (thread_info, format);
    if (!context)
	return NULL;

    drawable = _glitz_cgl_create_drawable (thread_info, context, format,
					   view, NULL,
					   width, height);
    if (!drawable)
	return NULL;

    return &drawable->base;
}
slim_hidden_def(glitz_cgl_create_drawable_for_window);

glitz_drawable_t *
glitz_cgl_create_pbuffer_drawable (glitz_drawable_format_t *format,
				   unsigned int            width,
				   unsigned int            height)
{
    glitz_cgl_thread_info_t *thread_info;

    thread_info = glitz_cgl_thread_info_get ();
    if (!thread_info)
	return NULL;

    if (format->id >= thread_info->n_formats)
	return NULL;

    return _glitz_cgl_create_pbuffer_drawable (thread_info, format,
					       width, height);
}
slim_hidden_def(glitz_cgl_create_pbuffer_drawable);

void
glitz_cgl_destroy (void *abstract_drawable)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    glitz_cgl_drawable_t *drawable = (glitz_cgl_drawable_t *)
	abstract_drawable;

    drawable->thread_info->drawables--;
    if (drawable->thread_info->drawables == 0) {
	/*
	 * Last drawable? We have to destroy all fragment programs as this may
	 * be our last chance to have a context current.
	 */
	glitz_cgl_push_current (abstract_drawable, NULL,
				GLITZ_CONTEXT_CURRENT, NULL);
	glitz_program_map_fini (drawable->base.backend->gl,
				&drawable->thread_info->program_map);
	glitz_program_map_init (&drawable->thread_info->program_map);
	glitz_cgl_pop_current (abstract_drawable);
    }

    if (drawable->view || drawable->pbuffer) {
	NSOpenGLContext *context = [NSOpenGLContext currentContext];

	if (context == drawable->context->context) {
	    if (drawable->pbuffer) {
		if ([context pixelBuffer] == drawable->pbuffer)
		    [NSOpenGLContext clearCurrentContext];
	    } else {
		if ([context view] == drawable->view)
		    [NSOpenGLContext clearCurrentContext];
	    }
	}

	if (drawable->pbuffer)
	    glitz_cgl_pbuffer_destroy (drawable->pbuffer);
    }

    [pool release];
    free (drawable);
}

glitz_bool_t
glitz_cgl_swap_buffers (void *abstract_drawable)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    glitz_cgl_drawable_t *drawable = (glitz_cgl_drawable_t *)
	abstract_drawable;

    glitz_cgl_push_current (abstract_drawable, NULL, GLITZ_DRAWABLE_CURRENT,
			    NULL);
    [drawable->context->context flushBuffer];
    glitz_cgl_pop_current (abstract_drawable);

    [pool release];
    return 1;
}

glitz_bool_t
glitz_cgl_copy_sub_buffer (void *abstract_drawable,
			   int  x,
			   int  y,
			   int  width,
			   int  height)
{
    return 0;
}
