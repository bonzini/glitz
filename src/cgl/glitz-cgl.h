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

#ifndef GLITZ_CGL_H_INCLUDED
#define GLITZ_CGL_H_INCLUDED

#include <glitz.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <OpenGL/OpenGL.h>

#ifdef __OBJC__
#include <AppKit/NSView.h>
#include <AppKit/NSOpenGL.h>
#endif


/* glitz_cgl_info.c */

void
glitz_cgl_init (void);

void
glitz_cgl_fini (void);


/* glitz_cgl_format.c */

glitz_drawable_format_t *
glitz_cgl_find_window_format (unsigned long                 mask,
			      const glitz_drawable_format_t *templ,
			      int                           count);

glitz_drawable_format_t *
glitz_cgl_find_pbuffer_format (unsigned long                 mask,
			       const glitz_drawable_format_t *templ,
			       int                           count);

/* glitz_cgl_drawable.c */

#ifdef __OBJC__
glitz_drawable_t *
glitz_cgl_create_drawable_for_view (glitz_drawable_format_t *format,
				    NSView                  *view,
				    unsigned int            width,
				    unsigned int            height);
#endif

glitz_drawable_t *
glitz_cgl_create_pbuffer_drawable (glitz_drawable_format_t *format,
				   unsigned int            width,
				   unsigned int            height);


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* GLITZ_CGL_H_INCLUDED */
