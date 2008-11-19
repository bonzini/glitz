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

#ifndef GLITZ_GLXINT_H_INCLUDED
#define GLITZ_GLXINT_H_INCLUDED

#include "glitz.h"
#include "glitzint.h"

#include "glitz-cgl.h"

#include <OpenGL/gl.h>
#include <AppKit/AppKit.h>

#define GLITZ_CGL_FEATURE_PBUFFER_MASK               (1L << 0)
#define GLITZ_CGL_FEATURE_MULTISAMPLE_MASK           (1L << 1)
#define GLITZ_CGL_FEATURE_PBUFFER_DOUBLEBUFFER_MASK  (1L << 2)
#define GLITZ_CGL_FEATURE_PBUFFER_MULTISAMPLE_MASK   (1L << 3)
#define GLITZ_CGL_FEATURE_PBUFFER_DEPTH_STENCIL_MASK (1L << 4)
#define GLITZ_CGL_FEATURE_TEXTURE_RECTANGLE_MASK     (1L << 5)

typedef struct _glitz_cgl_drawable glitz_cgl_drawable_t;

typedef struct _glitz_cgl_context_info_t {
    glitz_cgl_drawable_t *drawable;
    glitz_surface_t      *surface;
    glitz_constraint_t   constraint;
} glitz_cgl_context_info_t;

typedef struct _glitz_cgl_context_t {
    glitz_context_t     base;
    NSOpenGLContext     *context;
    glitz_format_id_t   id;
    NSOpenGLPixelFormat *pixel_format;
    glitz_bool_t        pbuffer;
    glitz_backend_t     backend;
    glitz_bool_t        initialized;
} glitz_cgl_context_t;

typedef struct _glitz_cgl_thread_info_t {
    int                         drawables;
    glitz_int_drawable_format_t *formats;
    NSOpenGLPixelFormat         **pixel_formats;
    int                         n_formats;
    glitz_cgl_context_t         **contexts;
    int                         n_contexts;
    glitz_cgl_context_info_t    context_stack[GLITZ_CONTEXT_STACK_SIZE];
    int                         context_stack_size;
    NSOpenGLContext             *root_context;
    unsigned long               cgl_feature_mask;
    glitz_context_t             *cctx;
    glitz_program_map_t         program_map;
} glitz_cgl_thread_info_t;

struct _glitz_cgl_drawable {
    glitz_drawable_t        base;

    glitz_cgl_thread_info_t *thread_info;
    glitz_cgl_context_t     *context;
    NSOpenGLPixelBuffer     *pbuffer;
    NSView		    *view;
    int                     width;
    int                     height;
};

extern glitz_status_t __internal_linkage
glitz_cgl_query_extensions (glitz_cgl_thread_info_t *thread_info);

extern glitz_cgl_thread_info_t __internal_linkage *
glitz_cgl_thread_info_get (void);

extern glitz_cgl_context_t __internal_linkage *
glitz_cgl_context_get (glitz_cgl_thread_info_t *thread_info,
		       glitz_drawable_format_t *format);

extern void __internal_linkage
glitz_cgl_context_destroy (glitz_cgl_thread_info_t *thread_info,
			   glitz_cgl_context_t     *context);

extern void __internal_linkage
glitz_cgl_query_formats (glitz_cgl_thread_info_t *thread_info);

extern NSOpenGLPixelBuffer *__internal_linkage
glitz_cgl_pbuffer_create (glitz_cgl_thread_info_t *thread_info,
			  int                     width,
			  int                     height);

extern void __internal_linkage
glitz_cgl_pbuffer_destroy (NSOpenGLPixelBuffer *pbuffer);

extern glitz_drawable_t __internal_linkage *
glitz_cgl_create_pbuffer (void                    *abstract_templ,
			  glitz_drawable_format_t *format,
			  unsigned int            width,
			  unsigned int            height);

extern glitz_bool_t __internal_linkage
glitz_cgl_push_current (void               *abstract_drawable,
			glitz_surface_t    *surface,
			glitz_constraint_t constraint,
			glitz_bool_t       *restore_state);

extern glitz_surface_t __internal_linkage *
glitz_cgl_pop_current (void *abstract_drawable);

extern void __internal_linkage
glitz_cgl_destroy (void *abstract_drawable);

extern glitz_bool_t __internal_linkage
glitz_cgl_swap_buffers (void *abstract_drawable);

extern glitz_bool_t __internal_linkage
glitz_cgl_copy_sub_buffer (void *abstract_drawable,
			   int  x,
			   int  y,
			   int  width,
			   int  height);

extern glitz_bool_t __internal_linkage
glitz_cgl_drawable_update_size (glitz_cgl_drawable_t *drawable,
				int                  width,
				int                  height);

/* Avoid unnecessary PLT entries.  */

slim_hidden_proto(glitz_cgl_init)
slim_hidden_proto(glitz_cgl_fini)
slim_hidden_proto(glitz_cgl_find_drawable_format)
slim_hidden_proto(glitz_cgl_create_drawable_for_window)
slim_hidden_proto(glitz_cgl_create_pbuffer_drawable)
slim_hidden_proto(glitz_cgl_drawable_update_size)

#endif /* GLITZ_GLXINT_H_INCLUDED */
