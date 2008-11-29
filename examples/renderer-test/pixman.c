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

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/XCB/xcb.h>
#include <pixman.h>

#include "rendertest.h"

typedef struct pixman_surface {
  XCBConnection *c;
  XCBDRAWABLE drawable;
  pixman_image_t *image;
} pixman_surface_t;

static pixman_operator_t
_pixman_operator (render_operator_t op)
{
  switch (op) {
  case RENDER_OPERATOR_CLEAR:
    return PIXMAN_OPERATOR_CLEAR;
  case RENDER_OPERATOR_SRC:
    return PIXMAN_OPERATOR_SRC;
  case RENDER_OPERATOR_DST:
    return PIXMAN_OPERATOR_DST;
  case RENDER_OPERATOR_OVER:
    return PIXMAN_OPERATOR_OVER;
  case RENDER_OPERATOR_OVER_REVERSE:
    return PIXMAN_OPERATOR_OVER_REVERSE;
  case RENDER_OPERATOR_IN:
    return PIXMAN_OPERATOR_IN;
  case RENDER_OPERATOR_IN_REVERSE:
    return PIXMAN_OPERATOR_IN_REVERSE;
  case RENDER_OPERATOR_OUT:
    return PIXMAN_OPERATOR_OUT;
  case RENDER_OPERATOR_OUT_REVERSE:
    return PIXMAN_OPERATOR_OUT_REVERSE;
  case RENDER_OPERATOR_ATOP:
    return PIXMAN_OPERATOR_ATOP;
  case RENDER_OPERATOR_ATOP_REVERSE:
    return PIXMAN_OPERATOR_ATOP_REVERSE;
  case RENDER_OPERATOR_XOR:
    return PIXMAN_OPERATOR_XOR;
  case RENDER_OPERATOR_ADD:
    return PIXMAN_OPERATOR_ADD;
  default:
    return PIXMAN_OPERATOR_OVER;
  }
}

static render_surface_t *
_pixman_render_create_similar (render_surface_t *other,
			       render_format_t render_format,
			       int width,
			       int height)
{
  pixman_surface_t *other_surface = (pixman_surface_t *) other->surface;
  render_surface_t *similar;
  pixman_surface_t *surface;
  pixman_format_t *format = 0;

  switch (render_format) {
  case RENDER_FORMAT_A1:
    format = pixman_format_create (PIXMAN_FORMAT_NAME_A1);
    break;
  case RENDER_FORMAT_A8:
    format = pixman_format_create (PIXMAN_FORMAT_NAME_A8);
    break;
  case RENDER_FORMAT_RGB24:
    format = pixman_format_create (PIXMAN_FORMAT_NAME_RGB24);
    break;
  case RENDER_FORMAT_ARGB32:
    format = pixman_format_create (PIXMAN_FORMAT_NAME_ARGB32);
  default:
      break;
  }

  if (format == NULL)
    return NULL;

  similar = malloc (sizeof (render_surface_t));
  if (similar == NULL)
    return NULL;

  similar->width = width;
  similar->height = height;
  similar->backend = other->backend;
  similar->flags = 0;

  surface = malloc (sizeof (pixman_surface_t));
  if (surface == NULL) {
    free (similar);
    return NULL;
  }

  surface->image = pixman_image_create (format, width, height);
  surface->drawable = other_surface->drawable;
  surface->c = NULL;

  pixman_format_destroy (format);

  if (surface->image == NULL) {
    free (similar);
    free (surface);
    return NULL;
  }

  similar->surface = surface;

  return similar;
}

static void
_pixman_render_destroy (render_surface_t *surface)
{
  pixman_surface_t *s = (pixman_surface_t *) surface->surface;

  pixman_image_destroy (s->image);

  free (s);
  free (surface);
}

static render_status_t
_pixman_render_composite (render_operator_t op,
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
			  int height)
{
  pixman_image_t *mask_image = NULL;

  if (mask)
    mask_image = ((pixman_surface_t *) mask->surface)->image;

  pixman_composite (_pixman_operator (op),
		    ((pixman_surface_t *) src->surface)->image,
		    mask_image,
		    ((pixman_surface_t *) dst->surface)->image,
		    x_src,
		    y_src,
		    x_mask,
		    y_mask,
		    x_dst,
		    y_dst,
		    width,
		    height);

  return RENDER_STATUS_SUCCESS;
}

static render_status_t
_pixman_render_set_pixels (render_surface_t *dst,
			   render_format_t format,
			   unsigned char *data)
{
  pixman_surface_t *s = (pixman_surface_t *) dst->surface;
  pixman_bits_t *bits;
  int len;

  switch (format) {
  case RENDER_FORMAT_A1:
    len = (((dst->width + 3) & -4) >> 3) * dst->height;
    break;
  case RENDER_FORMAT_A8:
    len = ((dst->width + 3) & -4) * dst->height;
    break;
  case RENDER_FORMAT_RGB24:
    len = 4 * dst->width * dst->height;
    break;
  case RENDER_FORMAT_ARGB32:
  default:
    len = 4 * dst->width * dst->height;
    break;
  }

  bits = pixman_image_get_data (s->image);
  if (!bits)
    return RENDER_STATUS_NO_MEMORY;

  memcpy (bits, data, len);

  return RENDER_STATUS_SUCCESS;
}

static void
_pixman_render_show (render_surface_t *surface)
{
  pixman_surface_t *s = (pixman_surface_t *) surface->surface;
  XCBGCONTEXT gc;
  pixman_bits_t *bits;

  if (s->c == NULL)
    return;

  gc = XCBGCONTEXTNew (s->c);
  XCBCreateGC (s->c, gc, s->drawable, 0, 0);

  bits = pixman_image_get_data (s->image);
  if (!bits)
    return;

  XCBPutImage (s->c, ZPixmap, s->drawable, gc,
	       surface->width,
	       surface->height,
	       0, 0, 0, 24,
	       4 * surface->width * surface->height,
	       (unsigned char *) bits);

  XCBFreeGC (s->c, gc);

  XCBSync (s->c, NULL);
}

static render_status_t
_pixman_render_set_fill (render_surface_t *surface,
			 render_fill_t fill)
{
  pixman_surface_t *s = (pixman_surface_t *) surface->surface;

  switch (fill) {
  case RENDER_FILL_TRANSPARENT:
    if (!SURFACE_TRANSFORM (surface))
      return RENDER_STATUS_NOT_SUPPORTED;
    pixman_image_set_repeat (s->image, 0);
    break;
  case RENDER_FILL_NEAREST:
  case RENDER_FILL_REFLECT:
    return RENDER_STATUS_NOT_SUPPORTED;
  case RENDER_FILL_REPEAT:
    pixman_image_set_repeat (s->image, 1);
    break;
  case RENDER_FILL_NONE:
  default:
    pixman_image_set_repeat (s->image, 0);
    break;
  }

  return RENDER_STATUS_SUCCESS;
}

static render_status_t
_pixman_render_set_component_alpha (render_surface_t *surface,
				    render_bool_t component_alpha)
{
  pixman_surface_t *s = (pixman_surface_t *) surface->surface;

  pixman_image_set_component_alpha (s->image, component_alpha);

  return RENDER_STATUS_SUCCESS;
}

static render_status_t
_pixman_render_set_transform (render_surface_t *surface,
			      render_matrix_t *matrix)
{
  pixman_transform_t transform;
  pixman_surface_t *s = (pixman_surface_t *) surface->surface;

  transform.matrix[0][0] = DOUBLE_TO_FIXED (matrix->m[0][0]);
  transform.matrix[0][1] = DOUBLE_TO_FIXED (matrix->m[0][1]);
  transform.matrix[0][2] = DOUBLE_TO_FIXED (matrix->m[0][2]);

  transform.matrix[1][0] = DOUBLE_TO_FIXED (matrix->m[1][0]);
  transform.matrix[1][1] = DOUBLE_TO_FIXED (matrix->m[1][1]);
  transform.matrix[1][2] = DOUBLE_TO_FIXED (matrix->m[1][2]);

  transform.matrix[2][0] = DOUBLE_TO_FIXED (matrix->m[2][0]);
  transform.matrix[2][1] = DOUBLE_TO_FIXED (matrix->m[2][1]);
  transform.matrix[2][2] = DOUBLE_TO_FIXED (matrix->m[2][2]);

  pixman_image_set_transform (s->image, &transform);

  return RENDER_STATUS_SUCCESS;
}

static render_status_t
_pixman_render_set_filter (render_surface_t *surface,
			   render_filter_t filter,
			   render_fixed16_16_t *params,
			   int n_params)
{
  pixman_surface_t *s = (pixman_surface_t *) surface->surface;

  switch (filter) {
  case RENDER_FILTER_NEAREST:
    pixman_image_set_filter (s->image, PIXMAN_FILTER_NEAREST);
    break;
  case RENDER_FILTER_BILINEAR:
    pixman_image_set_filter (s->image, PIXMAN_FILTER_BILINEAR);
    break;
  default:
    return RENDER_STATUS_NOT_SUPPORTED;
    break;
  }

  return RENDER_STATUS_SUCCESS;
}

static render_status_t
_pixman_render_set_clip_rectangles (render_surface_t *surface,
				    int x_offset,
				    int y_offset,
				    render_rectangle_t *rects,
				    int n_rects)
{
  pixman_surface_t *s = (pixman_surface_t *) surface->surface;
  pixman_region16_t *region;

  if (n_rects > 0) {
    region = pixman_region_create ();

    for (; n_rects--; rects++)
      pixman_region_union_rect (region, region,
				x_offset + rects->x, y_offset + rects->y,
				rects->width, rects->height);
  } else
    region = NULL;

  pixman_image_set_clip_region (s->image, region);

  if (region)
    pixman_region_destroy (region);

  return RENDER_STATUS_SUCCESS;
}

static render_status_t
_pixman_render_set_clip_trapezoids (render_surface_t *surface,
				    int x_offset,
				    int y_offset,
				    render_trapezoid_t *traps,
				    int n_traps)
{
  return RENDER_STATUS_NOT_SUPPORTED;
}

static const render_backend_t _pixman_render_backend = {
  _pixman_render_create_similar,
  _pixman_render_destroy,
  _pixman_render_composite,
  _pixman_render_set_pixels,
  _pixman_render_show,
  _pixman_render_set_fill,
  _pixman_render_set_component_alpha,
  _pixman_render_set_transform,
  _pixman_render_set_filter,
  _pixman_render_set_clip_rectangles,
  _pixman_render_set_clip_trapezoids
};

static const render_option_t _pixman_options[] = {
  { 0 }
};

static int
_parse_option (int key, char *arg, render_arg_state_t *state)
{
  return 1;
}

int
main (int argc, char **argv)
{
  int status, x, y;
  render_surface_t surface;
  render_arg_state_t state;
  pixman_surface_t dst;
  XCBSCREEN *root;
  XCBGenericEvent *xev;
  CARD32 mask;
  CARD32 values[2];
  pixman_format_t *format;

  state.pointer = NULL;

  if (render_parse_arguments (_parse_option,
			      _pixman_options,
			      &state,
			      argc, argv))
    return 1;

  surface.backend = &_pixman_render_backend;
  surface.flags = 0;

  x = y = 50;
  surface.width = RENDER_DEFAULT_DST_WIDTH;
  surface.height = RENDER_DEFAULT_DST_HEIGHT;

  format = pixman_format_create (PIXMAN_FORMAT_NAME_ARGB32);

  if (format == NULL)
    return 1;

  dst.image = pixman_image_create (format, surface.width, surface.height);
  if (dst.image == NULL) {
    pixman_format_destroy (format);
    return 1;
  }

  dst.c = XCBConnectBasic ();
  if (dst.c == NULL) {
    fprintf (stderr, "Error: can't open display\n");
    return 1;
  }

  root = XCBConnSetupSuccessRepRootsIter (XCBGetSetup (dst.c)).data;
  mask = XCBCWBackPixel | XCBCWEventMask;
  values[0] = root->black_pixel;
  values[1] = ExposureMask;

  dst.drawable.window = XCBWINDOWNew (dst.c);
  XCBCreateWindow (dst.c, 0, dst.drawable.window, root->root,
		   x, y, surface.width, surface.height, 0,
		   InputOutput,  root->root_visual,
		   mask, values);

  XCBMapWindow (dst.c, dst.drawable.window);
  XCBSync (dst.c, NULL);

  do {
    xev = XCBWaitEvent (dst.c);
    if (xev && xev->response_type == XCBExpose)
      break;
  } while (xev);

  surface.surface = &dst;

  status = render_run (&surface, &state.settings);

  XCBDestroyWindow (dst.c, dst.drawable.window);
  XCBDisconnect (dst.c);

  pixman_image_destroy (dst.image);
  pixman_format_destroy (format);

  return 0;
}
