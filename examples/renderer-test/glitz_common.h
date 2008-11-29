/*
 * Copyright © 2004 David Reveman
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the names of
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
 * Author: David Reveman <davidr@freedesktop.org>
 */

#include <glitz.h>

#define FIXED_TO_FLOAT(f) (((glitz_float_t) (f)) / 65536)

#define RENDER_GLITZ_SURFACE_FLAG_CLIP_MASK (1L << 16)

#define GLITZ_SURFACE_CLIP(surface) \
  ((surface)->flags & RENDER_GLITZ_SURFACE_FLAG_CLIP_MASK)

render_surface_t *
_glitz_render_create_similar (render_surface_t *other,
			      render_format_t format,
			      int width,
			      int height);

void
_glitz_render_destroy (render_surface_t *surface);

render_status_t
_glitz_render_composite (render_operator_t op,
			 render_surface_t *src,
			 render_surface_t *mask,
			 render_surface_t *dst,
			 int x_src,
			 int y_src,
			 int x_mask,
			 int y_mask,
			 int x_dst,
			 int y_dst,
			 int width,
			 int height);

render_status_t
_glitz_render_set_pixels (render_surface_t *dst,
			  render_format_t format,
			  unsigned char *data);

void
_glitz_render_show (render_surface_t *surface);

render_status_t
_glitz_render_set_fill (render_surface_t *surface,
			render_fill_t fill);

render_status_t
_glitz_render_set_component_alpha (render_surface_t *surface,
				   render_bool_t component_alpha);

render_status_t
_glitz_render_set_transform (render_surface_t *surface,
			     render_matrix_t *matrix);

render_status_t
_glitz_render_set_filter (render_surface_t *surface,
			  render_filter_t filter,
			  render_fixed16_16_t *params,
			  int n_params);

render_status_t
_glitz_render_set_clip_rectangles (render_surface_t *surface,
				   int x_offset,
				   int y_offset,
				   render_rectangle_t *rects,
				   int n_rects);

render_status_t
_glitz_render_set_clip_trapezoids (render_surface_t *surface,
				   int x_offset,
				   int y_offset,
				   render_trapezoid_t *traps,
				   int n_traps);

glitz_surface_t *
_glitz_create_surface_for_drawable (glitz_drawable_t *drawable,
				    int width,
				    int height);

glitz_surface_t *
_glitz_create_and_attach_surface_to_drawable (glitz_drawable_t *drawable,
					      glitz_drawable_t *attach,
					      int width,
					      int height);
