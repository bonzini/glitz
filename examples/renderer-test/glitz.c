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
#  include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include <glitz.h>

#include "rendertest.h"
#include "glitz_common.h"

static render_status_t
_glitz_status (glitz_status_t status)
{
  switch (status) {
  case GLITZ_STATUS_NOT_SUPPORTED:
    return RENDER_STATUS_NOT_SUPPORTED;
  case GLITZ_STATUS_NO_MEMORY:
    return RENDER_STATUS_NO_MEMORY;
  default:
    return RENDER_STATUS_SUCCESS;
  }
}

static glitz_operator_t
_glitz_operator (render_operator_t op)
{
  switch (op) {
  case RENDER_OPERATOR_CLEAR:
    return GLITZ_OPERATOR_CLEAR;
  case RENDER_OPERATOR_SRC:
    return GLITZ_OPERATOR_SRC;
  case RENDER_OPERATOR_DST:
    return GLITZ_OPERATOR_DST;
  case RENDER_OPERATOR_OVER:
    return GLITZ_OPERATOR_OVER;
  case RENDER_OPERATOR_OVER_REVERSE:
    return GLITZ_OPERATOR_OVER_REVERSE;
  case RENDER_OPERATOR_IN:
    return GLITZ_OPERATOR_IN;
  case RENDER_OPERATOR_IN_REVERSE:
    return GLITZ_OPERATOR_IN_REVERSE;
  case RENDER_OPERATOR_OUT:
    return GLITZ_OPERATOR_OUT;
  case RENDER_OPERATOR_OUT_REVERSE:
    return GLITZ_OPERATOR_OUT_REVERSE;
  case RENDER_OPERATOR_ATOP:
    return GLITZ_OPERATOR_ATOP;
  case RENDER_OPERATOR_ATOP_REVERSE:
    return GLITZ_OPERATOR_ATOP_REVERSE;
  case RENDER_OPERATOR_XOR:
    return GLITZ_OPERATOR_XOR;
  case RENDER_OPERATOR_ADD:
    return GLITZ_OPERATOR_ADD;
  default:
    return GLITZ_OPERATOR_OVER;
  }
}

static glitz_format_name_t
_glitz_format (render_format_t format)
{
  switch (format) {
  case RENDER_FORMAT_A1:
    return GLITZ_STANDARD_A1;
  case RENDER_FORMAT_A8:
    return GLITZ_STANDARD_A8;
  case RENDER_FORMAT_RGB24:
    return GLITZ_STANDARD_RGB24;
  case RENDER_FORMAT_ARGB32:
  default:
    return GLITZ_STANDARD_ARGB32;
  }
}

static glitz_fill_t
_glitz_fill (render_fill_t fill)
{
  switch (fill) {
  case RENDER_FILL_REPEAT:
    return GLITZ_FILL_REPEAT;
  case RENDER_FILL_REFLECT:
    return GLITZ_FILL_REFLECT;
  case RENDER_FILL_TRANSPARENT:
    return GLITZ_FILL_TRANSPARENT;
  case RENDER_FILL_NONE:
  case RENDER_FILL_NEAREST:
  default:
    return GLITZ_FILL_NEAREST;
  }
}

static glitz_filter_t
_glitz_filter (render_filter_t filter)
{
  switch (filter) {
  case RENDER_FILTER_BILINEAR:
    return GLITZ_FILTER_BILINEAR;
  case RENDER_FILTER_CONVOLUTION:
    return GLITZ_FILTER_CONVOLUTION;
  case RENDER_FILTER_GAUSSIAN:
    return GLITZ_FILTER_GAUSSIAN;
  case RENDER_FILTER_LINEAR_GRADIENT:
    return GLITZ_FILTER_LINEAR_GRADIENT;
  case RENDER_FILTER_RADIAL_GRADIENT:
    return GLITZ_FILTER_RADIAL_GRADIENT;
  default:
    return GLITZ_FILTER_NEAREST;
  }
}

render_surface_t *
_glitz_render_create_similar (render_surface_t *other,
			      render_format_t render_format,
			      int width,
			      int height)
{
  render_surface_t *similar;
  glitz_drawable_t *drawable;
  glitz_format_t *format = NULL;

  drawable = glitz_surface_get_drawable ((glitz_surface_t *) other->surface);

  switch (render_format) {
  case RENDER_FORMAT_A1:
  case RENDER_FORMAT_A8:
  case RENDER_FORMAT_RGB24:
  case RENDER_FORMAT_ARGB32:
      format = glitz_find_standard_format (drawable,
					   _glitz_format (render_format));
      break;
  case RENDER_FORMAT_YV12: {
      glitz_format_t templ;

      templ.color.fourcc = GLITZ_FOURCC_YV12;
      format = glitz_find_format (drawable, GLITZ_FORMAT_FOURCC_MASK,
                                  &templ, 0);
  } break;
  case RENDER_FORMAT_YUY2: {
      glitz_format_t templ;

      templ.color.fourcc = GLITZ_FOURCC_YUY2;
      format = glitz_find_format (drawable, GLITZ_FORMAT_FOURCC_MASK,
                                  &templ, 0);
  } break;
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

  similar->surface = (glitz_surface_t *)
    glitz_surface_create (drawable, format, width, height, 0, NULL);
  if (similar->surface == NULL) {
    free (similar);
    return NULL;
  }

  return similar;
}

void
_glitz_render_destroy (render_surface_t *surface)
{
  glitz_surface_destroy ((glitz_surface_t *) surface->surface);
  free (surface);
}

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

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
			 int height)
{
  glitz_status_t status;

  if (GLITZ_SURFACE_CLIP (src)) {
    if (x_src < 0) {
      x_dst -= x_src;
      x_mask -= x_src;
      width += x_src;
      x_src = 0;
    }

    if (y_src < 0) {
      y_dst -= y_src;
      y_mask -= y_src;
      height += y_src;
      y_src = 0;
    }

    width = MIN (src->width - x_src, width);
    height = MIN (src->height - y_src, height);
  }

  if (mask && GLITZ_SURFACE_CLIP (mask)) {
    if (x_mask < 0) {
      x_dst -= x_mask;
      x_src -= x_mask;
      width += x_mask;
      x_mask = 0;
    }

    if (y_mask < 0) {
      y_dst -= y_mask;
      y_src -= y_mask;
      height += y_mask;
      y_mask = 0;
    }

    width = MIN (mask->width - x_mask, width);
    height = MIN (mask->height - y_mask, height);
  }

  glitz_composite (_glitz_operator (op),
		   (glitz_surface_t *) src->surface,
		   (glitz_surface_t *) ((mask) ? mask->surface: NULL),
		   (glitz_surface_t *) dst->surface,
		   x_src,
		   y_src,
		   x_mask,
		   y_mask,
		   x_dst,
		   y_dst,
		   width,
		   height);

  status = glitz_surface_get_status ((glitz_surface_t *) dst->surface);
  while (glitz_surface_get_status ((glitz_surface_t *) dst->surface));

  return _glitz_status (status);
}

#undef MIN
#undef MAX

render_status_t
_glitz_render_set_pixels (render_surface_t *dst,
			  render_format_t format,
			  unsigned char *data)
{
  glitz_status_t status;

  glitz_buffer_t *buffer;
  glitz_pixel_format_t pf;
  glitz_drawable_t *drawable;

  drawable = glitz_surface_get_drawable ((glitz_surface_t *) dst->surface);

  pf.scanline_order = GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN;
  pf.xoffset = 0;
  pf.skip_lines = 0;
  pf.fourcc = GLITZ_FOURCC_RGB;

  switch (format) {
  case RENDER_FORMAT_ARGB32:
    pf.masks.bpp = 32;
    pf.masks.alpha_mask = 0xff000000;
    pf.masks.red_mask =   0x00ff0000;
    pf.masks.green_mask = 0x0000ff00;
    pf.masks.blue_mask =  0x000000ff;
    break;
  case RENDER_FORMAT_RGB24:
    pf.masks.bpp = 32;
    pf.masks.alpha_mask = 0x00000000;
    pf.masks.red_mask =   0x00ff0000;
    pf.masks.green_mask = 0x0000ff00;
    pf.masks.blue_mask =  0x000000ff;
    break;
  case RENDER_FORMAT_A8:
    pf.masks.bpp = 8;
    pf.masks.alpha_mask = 0xff;
    pf.masks.red_mask =   0x00;
    pf.masks.green_mask = 0x00;
    pf.masks.blue_mask =  0x00;
    break;
  case RENDER_FORMAT_A1:
    pf.masks.bpp = 1;
    pf.masks.alpha_mask = 0x1;
    pf.masks.red_mask =   0x0;
    pf.masks.green_mask = 0x0;
    pf.masks.blue_mask =  0x0;
    break;
  case RENDER_FORMAT_YV12:
    pf.fourcc = GLITZ_FOURCC_YV12;
    pf.masks.bpp = 12;
    pf.masks.alpha_mask = 0x0;
    pf.masks.red_mask =   0x0;
    pf.masks.green_mask = 0x0;
    pf.masks.blue_mask =  0x0;
    break;
  case RENDER_FORMAT_YUY2:
    pf.fourcc = GLITZ_FOURCC_YUY2;
    pf.masks.bpp = 16;
    pf.masks.alpha_mask = 0x0;
    pf.masks.red_mask =   0x0;
    pf.masks.green_mask = 0x0;
    pf.masks.blue_mask =  0x0;
    break;
  }

  pf.bytes_per_line = (((dst->width * pf.masks.bpp) / 8) + 3) & -4;

  buffer = glitz_pixel_buffer_create (drawable,
				      data,
				      pf.bytes_per_line * dst->height,
				      GLITZ_BUFFER_HINT_STREAM_DRAW);

  glitz_set_pixels ((glitz_surface_t *) dst->surface,
		    0, 0,
		    dst->width, dst->height,
		    &pf,
		    buffer);

  glitz_buffer_destroy (buffer);

  status = glitz_surface_get_status ((glitz_surface_t *) dst->surface);
  while (glitz_surface_get_status ((glitz_surface_t *) dst->surface));

  return _glitz_status (status);
}

void
_glitz_render_show (render_surface_t *surface)
{
    glitz_drawable_t *drawable, *attached;
    glitz_surface_t *s = (glitz_surface_t *) surface->surface;
    glitz_drawable_format_t *dformat;
    int width, height;

    drawable = glitz_surface_get_drawable (s);
    attached = glitz_surface_get_attached_drawable (s);
    dformat  = glitz_drawable_get_format (drawable);

    width  = glitz_surface_get_width (s);
    height = glitz_surface_get_height (s);

    if (attached != drawable)
    {
	glitz_format_t *format;

	format = glitz_find_standard_format (drawable, GLITZ_STANDARD_ARGB32);
	if (format) {
	    glitz_surface_t *dst;

	    dst = glitz_surface_create (drawable, format,
					width, height, 0, NULL);
	    if (dst)
	    {
		if (dformat->doublebuffer)
		    glitz_surface_attach (dst, drawable,
					  GLITZ_DRAWABLE_BUFFER_BACK_COLOR);
		else
		    glitz_surface_attach (dst, drawable,
					  GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);

		glitz_copy_area (s, dst, 0, 0, width, height, 0, 0);

		glitz_surface_flush (dst);
		glitz_surface_destroy (dst);
	    }
	}
    }

    if (dformat->doublebuffer)
	glitz_drawable_swap_buffers (drawable);
    else
	glitz_drawable_finish (drawable);
}

render_status_t
_glitz_render_set_fill (render_surface_t *surface,
			render_fill_t fill)
{
  if (fill == RENDER_FILL_NONE)
  {
    glitz_format_t *format;

    format = glitz_surface_get_format ((glitz_surface_t *) surface->surface);
    if (format->color.fourcc != GLITZ_FOURCC_RGB)
        fill = RENDER_FILL_NEAREST;

    surface->flags |= RENDER_GLITZ_SURFACE_FLAG_CLIP_MASK;
  }
  else
    surface->flags &= ~RENDER_GLITZ_SURFACE_FLAG_CLIP_MASK;

  glitz_surface_set_fill ((glitz_surface_t *) surface->surface,
			  _glitz_fill (fill));

  return RENDER_STATUS_SUCCESS;
}

render_status_t
_glitz_render_set_component_alpha (render_surface_t *surface,
				   render_bool_t component_alpha)
{
  glitz_surface_set_component_alpha ((glitz_surface_t *) surface->surface,
				     component_alpha);

  return RENDER_STATUS_SUCCESS;
}

render_status_t
_glitz_render_set_transform (render_surface_t *surface,
			     render_matrix_t *matrix)
{
  glitz_transform_t transform;

  transform.matrix[0][0] = DOUBLE_TO_FIXED (matrix->m[0][0]);
  transform.matrix[0][1] = DOUBLE_TO_FIXED (matrix->m[0][1]);
  transform.matrix[0][2] = DOUBLE_TO_FIXED (matrix->m[0][2]);

  transform.matrix[1][0] = DOUBLE_TO_FIXED (matrix->m[1][0]);
  transform.matrix[1][1] = DOUBLE_TO_FIXED (matrix->m[1][1]);
  transform.matrix[1][2] = DOUBLE_TO_FIXED (matrix->m[1][2]);

  transform.matrix[2][0] = DOUBLE_TO_FIXED (matrix->m[2][0]);
  transform.matrix[2][1] = DOUBLE_TO_FIXED (matrix->m[2][1]);
  transform.matrix[2][2] = DOUBLE_TO_FIXED (matrix->m[2][2]);

  glitz_surface_set_transform ((glitz_surface_t *) surface->surface,
			       &transform);

  return RENDER_STATUS_SUCCESS;
}

render_status_t
_glitz_render_set_filter (render_surface_t *surface,
			  render_filter_t filter,
			  render_fixed16_16_t *params,
			  int n_params)
{
  glitz_surface_set_filter ((glitz_surface_t *) surface->surface,
			    _glitz_filter (filter),
			    params, n_params);

  return RENDER_STATUS_SUCCESS;
}

render_status_t
_glitz_render_set_clip_rectangles (render_surface_t *surface,
				   int x_offset,
				   int y_offset,
				   render_rectangle_t *rects,
				   int n_rects)
{
  if (n_rects > 0) {
    glitz_float_t *data;
    glitz_buffer_t *buffer;
    glitz_drawable_t *drawable;
    glitz_geometry_format_t gf;
    int count;

    gf.vertex.primitive = GLITZ_PRIMITIVE_QUADS;
    gf.vertex.type = GLITZ_DATA_TYPE_FLOAT;
    gf.vertex.bytes_per_vertex = sizeof (glitz_float_t) * 2;
    gf.vertex.attributes = 0;

    count = n_rects * 4;

    drawable =
      glitz_surface_get_drawable ((glitz_surface_t *) surface->surface);

    buffer =
      glitz_vertex_buffer_create (drawable, NULL,
				  n_rects * 8 * sizeof (glitz_float_t),
				  GLITZ_BUFFER_HINT_STATIC_DRAW);
    if (!buffer)
      return RENDER_STATUS_NO_MEMORY;

    data = glitz_buffer_map (buffer, GLITZ_BUFFER_ACCESS_WRITE_ONLY);
    for (; n_rects; rects++, n_rects--) {
      *data++ = (glitz_float_t) rects->x;
      *data++ = (glitz_float_t) rects->y;
      *data++ = (glitz_float_t) (rects->x + rects->width);
      *data++ = (glitz_float_t) rects->y;
      *data++ = (glitz_float_t) (rects->x + rects->width);
      *data++ = (glitz_float_t) (rects->y + rects->height);
      *data++ = (glitz_float_t) rects->x;
      *data++ = (glitz_float_t) (rects->y + rects->height);
    }
    glitz_buffer_unmap (buffer);

    glitz_set_geometry ((glitz_surface_t *) surface->surface,
			GLITZ_GEOMETRY_TYPE_VERTEX,
			&gf, buffer);
    glitz_set_array ((glitz_surface_t *) surface->surface,
		     0, 2, count, x_offset << 16, y_offset << 16);

    glitz_buffer_destroy (buffer);
  } else
    glitz_set_geometry ((glitz_surface_t *) surface->surface,
			GLITZ_GEOMETRY_TYPE_NONE, NULL, NULL);

  return RENDER_STATUS_SUCCESS;
}

render_status_t
_glitz_render_set_clip_trapezoids (render_surface_t *surface,
				   int x_offset,
				   int y_offset,
				   render_trapezoid_t *traps,
				   int n_traps)
{
  if (n_traps > 0) {
    glitz_float_t *data;
    glitz_buffer_t *buffer;
    glitz_drawable_t *drawable;
    glitz_geometry_format_t gf;
    int count;

    gf.vertex.primitive = GLITZ_PRIMITIVE_QUADS;
    gf.vertex.type = GLITZ_DATA_TYPE_FLOAT;
    gf.vertex.bytes_per_vertex = sizeof (glitz_float_t) * 2;
    gf.vertex.attributes = 0;

    count = n_traps * 4;

    drawable =
      glitz_surface_get_drawable ((glitz_surface_t *) surface->surface);

    buffer =
      glitz_vertex_buffer_create (drawable, NULL,
				  n_traps * 8 * sizeof (glitz_float_t),
				  GLITZ_BUFFER_HINT_STATIC_DRAW);
    if (!buffer)
      return RENDER_STATUS_NO_MEMORY;

    data = glitz_buffer_map (buffer, GLITZ_BUFFER_ACCESS_WRITE_ONLY);
    for (; n_traps; traps++, n_traps--) {
      *data++ = FIXED_TO_FLOAT (traps->top.left);
      *data++ = FIXED_TO_FLOAT (traps->top.y);
      *data++ = FIXED_TO_FLOAT (traps->top.right);
      *data++ = FIXED_TO_FLOAT (traps->top.y);
      *data++ = FIXED_TO_FLOAT (traps->bottom.right);
      *data++ = FIXED_TO_FLOAT (traps->bottom.y);
      *data++ = FIXED_TO_FLOAT (traps->bottom.left);
      *data++ = FIXED_TO_FLOAT (traps->bottom.y);
    }
    glitz_buffer_unmap (buffer);

    glitz_set_geometry ((glitz_surface_t *) surface->surface,
			GLITZ_GEOMETRY_TYPE_VERTEX,
			&gf, buffer);
    glitz_set_array ((glitz_surface_t *) surface->surface,
		     0, 2, count, x_offset << 16, y_offset << 16);

    glitz_buffer_destroy (buffer);
  } else
    glitz_set_geometry ((glitz_surface_t *) surface->surface,
			GLITZ_GEOMETRY_TYPE_NONE, NULL, NULL);

  return RENDER_STATUS_SUCCESS;
}

glitz_surface_t *
_glitz_create_and_attach_surface_to_drawable (glitz_drawable_t *drawable,
					      glitz_drawable_t *attach,
					      int width,
					      int height)
{
  glitz_drawable_format_t *dformat;
  glitz_format_t *format, templ;
  glitz_surface_t *surface;
  glitz_drawable_buffer_t buffer;

  dformat = glitz_drawable_get_format (attach);

  templ.color = dformat->color;
  format = glitz_find_format (drawable,
			      GLITZ_FORMAT_FOURCC_MASK     |
			      GLITZ_FORMAT_RED_SIZE_MASK   |
			      GLITZ_FORMAT_GREEN_SIZE_MASK |
			      GLITZ_FORMAT_BLUE_SIZE_MASK  |
			      GLITZ_FORMAT_ALPHA_SIZE_MASK,
			      &templ,
			      0);
  if (!format) {
    fprintf (stderr, "Error: couldn't find surface format\n");
    return NULL;
  }

  surface = glitz_surface_create (drawable, format, width, height, 0, NULL);
  if (!surface) {
    fprintf (stderr, "Error: couldn't create glitz surface\n");
    return NULL;
  }

  if (dformat->doublebuffer)
    buffer = GLITZ_DRAWABLE_BUFFER_BACK_COLOR;
  else
    buffer = GLITZ_DRAWABLE_BUFFER_FRONT_COLOR;

  glitz_surface_attach (surface, attach, buffer);

  return surface;
}
