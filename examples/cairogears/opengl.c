/*
 * Copyright © 2008 Paolo Bonzini
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the names of
 * Paolo Bonzini not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission. Paolo Bonzini
 * makes no representations about the suitability of this software for
 * any purpose. It is provided "as is" without express or implied warranty.
 *
 * PAOLO BONZINI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL PAOLO BONZINI BE LIABLE FOR ANY SPECIAL, INDIRECT
 * OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Paolo Bonzini <bonzini@gnu.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cairo.h>
#include <math.h>

#include "cairogears.h"
#include "gears.h"
#include "glitz.h"

#define GEARS_SIZE	300

static cairo_surface_t *bg_surface;
static cairo_surface_t *gl_surface;
static glitz_context_t *gears_context;
static glitz_drawable_t *gears_drawable;
static struct gears *gears;
static int bg_width, bg_height;

static double angle;

cairo_t *cr;

void
opengl_setup (cairo_t *_cr, int w, int h)
{
  glitz_surface_t *gears_surface;
  glitz_drawable_format_t templ, *dformat;
  glitz_format_t *format;
  long mask;

  bg_surface = cairo_glitz_surface_create_from_png (_cr, "orion.png",
						    &bg_width, &bg_height);
  if (cairo_surface_status (bg_surface)) {
    printf ("error reading bg.png\n");
    exit(1);
  }

  format = glitz_find_standard_format (drawable, GLITZ_STANDARD_ARGB32);
  gears_surface = glitz_surface_create (drawable, format, GEARS_SIZE, GEARS_SIZE, 0, NULL);

  templ.color        = format->color;
  templ.depth_size   = 16;
  templ.doublebuffer = 0;

  mask =
    GLITZ_FORMAT_RED_SIZE_MASK     |
    GLITZ_FORMAT_GREEN_SIZE_MASK   |
    GLITZ_FORMAT_BLUE_SIZE_MASK    |
    GLITZ_FORMAT_ALPHA_SIZE_MASK   |
    GLITZ_FORMAT_DOUBLEBUFFER_MASK |
    GLITZ_FORMAT_DEPTH_SIZE_MASK;

  dformat = glitz_find_drawable_format (drawable, mask, &templ, 0);
  gears_drawable = glitz_create_drawable (drawable, dformat, GEARS_SIZE, GEARS_SIZE);
  glitz_surface_attach (gears_surface, gears_drawable, GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);
  glitz_drawable_destroy (gears_drawable);

  gl_surface = cairo_glitz_surface_create (gears_surface);
  glitz_surface_destroy (gears_surface);
  if (cairo_surface_status (gl_surface)) {
    printf ("error creating glitz surface\n");
    exit(1);
  }

  gears_context = glitz_context_create (drawable, glitz_drawable_get_format (drawable));
  glitz_context_make_current (gears_context, gears_drawable);
  gears = gears_create (0.0, 0.0, 0.0, 0.0);

  cr = _cr;
}

/* TODO: add a more cool effect! :-) */
double x = 0;
double dx = 0.005;

static void
paint_surface (cairo_t *cr, cairo_surface_t *src, int src_width, int src_height,
	       double dst_x, double dst_y, double dst_width, double dst_height,
	       double alpha)
{
  cairo_pattern_t *pattern;
  cairo_matrix_t matrix;

  double scale_x = src_width/dst_width;
  double scale_y = src_height/dst_height;

  /* translate -dst_x/-dst_y;
     scale src_width/dst_width, src_height/dst_height */
  cairo_matrix_init (&matrix, scale_x, 0, 0, scale_y,
		     -dst_x * scale_x, -dst_y * scale_y);

  pattern = cairo_pattern_create_for_surface (src);

  cairo_pattern_set_extend (pattern, CAIRO_EXTEND_NONE);
  cairo_pattern_set_matrix (pattern, &matrix);
  cairo_set_source (cr, pattern);
  cairo_pattern_destroy (pattern);

  /* TODO: make sure that neither of these require fallbacks.  */
  if (alpha == 1.0)
    {
      cairo_rectangle (cr, dst_x, dst_y, dst_x + dst_width, dst_y + dst_height);
      cairo_fill (cr);
    }
  else
    {
      cairo_save (cr);
      cairo_rectangle (cr, dst_x, dst_y, dst_x + dst_width, dst_y + dst_height);
      cairo_clip (cr);
      cairo_paint_with_alpha (cr, alpha);
      cairo_restore (cr);
    }
}

void
opengl_render (int w, int h)
{
  angle += 0.1;
  glitz_context_make_current (gears_context, gears_drawable);
  gears_draw (gears, angle);

  cairo_save (cr);
  cairo_scale (cr, w, h);

  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);
  cairo_rectangle (cr, 0.0, 0.0, 1.0, 1.0);
  cairo_fill (cr);

  paint_surface (cr, bg_surface, bg_width, bg_height, 0.5, 0.0, 0.5, 0.5, 1.0);
  paint_surface (cr, bg_surface, bg_width, bg_height, 0.0, 0.5, 0.5, 0.5, 1.0);

  if (x + dx > 0.25 || x + dx < 0)
    dx = -dx;
  x += dx;

  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  paint_surface (cr, gl_surface, GEARS_SIZE, GEARS_SIZE, 0.0, 0.0, 0.5, 0.5, 1.0);
  paint_surface (cr, gl_surface, GEARS_SIZE, GEARS_SIZE, 0.5, 0.0, 0.5, 0.5, 1.0);
  paint_surface (cr, gl_surface, GEARS_SIZE, GEARS_SIZE, 0.0, 0.5, 0.5, 0.5, 0.5);
  paint_surface (cr, gl_surface, GEARS_SIZE, GEARS_SIZE, x + 0.5, 0.625, 0.25, 0.25, 1.0);

  cairo_restore (cr);
}
