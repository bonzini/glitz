/*
 * Copyright � 2004 David Reveman, Peter Nilsson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the names of
 * David Reveman and Peter Nilsson not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission. David Reveman and Peter Nilsson
 * makes no representations about the suitability of this software for
 * any purpose. It is provided "as is" without express or implied warranty.
 *
 * DAVID REVEMAN AND PETER NILSSON DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL DAVID REVEMAN AND
 * PETER NILSSON BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: David Reveman <c99drn@cs.umu.se>
 *          Peter Nilsson <c99pnn@cs.umu.se>
 */

#ifndef GLITZ_AGLINT_H_INCLUDED
#define GLITZ_AGLINT_H_INCLUDED

#include "glitzint.h"

#include "glitz-agl.h"

#include <OpenGL/gl.h>
#include <Carbon/Carbon.h>
#include <AGL/agl.h>

#define GLITZ_AGL_FEATURE_PBUFFER_MASK                 (1L << 0)
#define GLITZ_AGL_FEATURE_TEXTURE_RECTANGLE_MASK       (1L << 1)
#define GLITZ_AGL_FEATURE_TEXTURE_NPOT_MASK            (1L << 2)
#define GLITZ_AGL_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK (1L << 3)
#define GLITZ_AGL_FEATURE_MULTISAMPLE_MASK             (1L << 4)
#define GLITZ_AGL_FEATURE_MULTISAMPLE_FILTER_MASK      (1L << 5)
#define GLITZ_AGL_FEATURE_ARB_MULTITEXTURE_MASK        (1L << 6)
#define GLITZ_AGL_FEATURE_ARB_VERTEX_PROGRAM_MASK      (1L << 7)
#define GLITZ_AGL_FEATURE_ARB_FRAGMENT_PROGRAM_MASK    (1L << 8)

typedef struct _glitz_agl_surface_t glitz_agl_surface_t;

typedef struct _glitz_agl_context_info_t {
  glitz_agl_surface_t *surface;
  glitz_constraint_t constraint;
} glitz_agl_context_info_t;

typedef struct _glitz_agl_context_t {
  AGLContext context;
  AGLPixelFormat pixel_format;
  glitz_bool_t offscreen;
  glitz_gl_uint_t texture_indirections;
} glitz_agl_context_t;

typedef struct _glitz_agl_thread_info_t {
  glitz_format_t *formats;
  AGLPixelFormat *format_ids;
  int n_formats;
  
  glitz_agl_context_t **contexts;
  int n_contexts;
  
  glitz_agl_context_info_t *context_stack;
  int context_stack_size;
  
  glitz_agl_context_t root_context;

  long int feature_mask;
  long int agl_feature_mask;
  long int texture_mask;

  glitz_programs_t programs;
} glitz_agl_thread_info_t;

struct _glitz_agl_surface_t {
  glitz_surface_t base;
  
  glitz_agl_thread_info_t *thread_info;
  glitz_agl_context_t *context;
  AGLDrawable drawable;
  AGLPbuffer pbuffer;
  WindowRef window;
};

extern void __internal_linkage
glitz_agl_query_extensions (glitz_agl_thread_info_t *thread_info);

extern glitz_agl_thread_info_t *__internal_linkage
glitz_agl_thread_info_get (void);

extern glitz_agl_context_t *__internal_linkage
glitz_agl_context_get (glitz_agl_thread_info_t *thread_info,
                       glitz_format_t *format,
                       glitz_bool_t offscreen);

extern void __internal_linkage
glitz_agl_context_make_current (glitz_agl_surface_t *surface);

extern glitz_agl_surface_t *__internal_linkage
glitz_agl_context_push_current (glitz_agl_surface_t *surface,
                                glitz_constraint_t constraint);

extern glitz_agl_surface_t *__internal_linkage
glitz_agl_context_pop_current (glitz_agl_surface_t *surface);

extern void __internal_linkage
glitz_agl_query_formats (glitz_agl_thread_info_t *thread_info);

extern AGLPbuffer __internal_linkage
glitz_agl_pbuffer_create (glitz_texture_t *texture);

extern void __internal_linkage
glitz_agl_pbuffer_bind (AGLPbuffer pbuffer,
                        AGLContext context,
                        glitz_texture_t *texture,
                        glitz_format_t *format);

extern void __internal_linkage
glitz_agl_pbuffer_destroy (AGLPbuffer pbuffer);


/* Avoid unnecessary PLT entries.  */

slim_hidden_proto(glitz_agl_find_format)
slim_hidden_proto(glitz_agl_find_standard_format)
slim_hidden_proto(glitz_agl_surface_create)
slim_hidden_proto(glitz_agl_surface_create_for_window)

#endif /* GLITZ_AGLINT_H_INCLUDED */
