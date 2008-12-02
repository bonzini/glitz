/*
 * Copyright © 2004 David Reveman, Peter Nilsson
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

#include <stdio.h>
#include <stdlib.h>
#include <cairo.h>
#include <math.h>

#include "cairogears.h"

cairo_surface_t *bg_surface;
cairo_surface_t *comp_surface;
int bg_width, bg_height;
int comp_width, comp_height;

cairo_t *cr;

void
comp_setup (cairo_t *_cr, int w, int h)
{
  bg_surface = cairo_glitz_surface_create_from_png (_cr, "bg.png",
						    &bg_width, &bg_height);
  if (cairo_surface_status (bg_surface)) {
    printf ("error reading bg.png\n");
    exit(1);
  }

  comp_surface = cairo_glitz_surface_create_from_png (_cr, "fg.png",
						      &comp_width, &comp_height);
  if (cairo_surface_status (comp_surface)) {
    printf ("error reading fg.png\n");
    exit(1);
  }

  cr = _cr;
}

double comp_x = 400 - 256 / 2;
double comp_y = 200 - 256 / 2;
double comp_x_dir = 5.0;
double comp_y_dir = 5.0;

double oversize = 1.0;
double oversize_dir = 0.001;

void
comp_render (int w, int h)
{
  double scale_x, scale_y;
  
  cairo_save (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_move_to (cr, 0, 0);

  scale_x = ((double) w) / (double) bg_width;
  scale_y = ((double) h) / (double) bg_height;

  cairo_translate (cr,
                   -((oversize - 1.0) * (double) w) / 2.0,
                   -((oversize - 1.0) * (double) h) / 2.0);
  
  cairo_scale (cr, scale_x * oversize, scale_y * oversize);

  cairo_set_source_surface (cr, bg_surface, 0.0, 0.0);
  cairo_paint (cr);
  cairo_restore (cr);

  oversize += oversize_dir;
  if (oversize >= 1.2)
    oversize_dir = -oversize_dir;
  if (oversize <= 1.0)
    oversize_dir = -oversize_dir;

  cairo_save (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  cairo_move_to (cr, 0, 0);

  if (comp_x_dir < 0) {
    if (comp_x_dir < -1.0)
      comp_x_dir += 10.0 / w;
  } else {
    if (comp_x_dir > 1.0)
      comp_x_dir -= 10.0 / w;
  }

  if (comp_y_dir < 0) {
  if (comp_y_dir < -1.0)
  comp_y_dir += 10.0 / h;
  } else {
  if (comp_y_dir > 1.0)
  comp_y_dir -= 10.0 / h;
  }

  comp_x += comp_x_dir;
  comp_y += comp_y_dir;

  if (comp_x >= (w - comp_width))
  comp_x_dir = -(drand48 () * 5.0 + 1.0 + comp_x_dir / 2.0);
  else if (comp_x <= 0)
  comp_x_dir = (drand48 () * 5.0 + 1.0 + comp_x_dir / 2.0);
  
  if (comp_y >= (h - comp_height))
  comp_y_dir = -(drand48 () * 5.0 + 1.0 + comp_x_dir / 2.0);
  else if (comp_y <= 0)
  comp_y_dir = drand48 () * 5.0 + 1.0 + comp_x_dir / 2.0;

  cairo_translate (cr, (int) comp_x, (int) comp_y);
  cairo_move_to (cr, 0.0, 0.0);

  cairo_set_source_surface (cr, comp_surface, 0.0, 0.0);
  cairo_paint (cr);
  cairo_restore (cr);
}
