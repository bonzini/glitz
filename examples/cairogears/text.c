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

#include <cairo.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "read_png.h"

float font_scale1, font_scale2,
  font_rotate1, font_rotate2,
  font_alpha1, font_alpha2,
  font_speed1, font_speed2,
  font_x1, font_y1,
  font_x2, font_y2;

cairo_surface_t *bg_surface;
int bg_width, bg_height;

cairo_t *cr;

void
text_setup (cairo_t *_cr, int w, int h)
{
  int image_width, image_height;
  char *image_data;
  cairo_surface_t *image;
  
  if (read_png ("orion.png", &image_data, &image_width, &image_height)) {
    printf ("error reading orion.png\n");
    exit(1);
  }

  bg_width = image_width;
  bg_height = image_height;

  image = cairo_image_surface_create_for_data (image_data, 
					       CAIRO_FORMAT_ARGB32,
					       image_width, image_height,
					       image_width * 4);

  bg_surface = cairo_surface_create_similar (cairo_get_target (_cr),
					     CAIRO_CONTENT_COLOR_ALPHA,
					     image_width, image_height);
  
  cr = cairo_create (bg_surface);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_surface (cr, image, 0.0, 0.0);
  cairo_paint (cr);
  cairo_destroy (cr);
  
  cairo_surface_destroy (image);
  free (image_data);

  cr = _cr;

  cairo_select_font_face (cr, 
			  "arial", 
			  CAIRO_FONT_SLANT_NORMAL, 
			  CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size (cr, 1.0);

  drand48 ();
  drand48 ();
  
  font_rotate1 = (float) (drand48 () * 3.14);
  font_rotate2 = (float) (drand48 () * 3.14);
  font_scale1 =  (float) (drand48 () * 250.0);
  font_scale2 =  (float) (drand48 () * 250.0);
  font_speed1 = (float) (drand48 () + 0.5);
  font_speed2 = (float) (drand48 () + 0.5);
  font_alpha1 = 0.0;
  font_alpha2 = 0.0;
  font_x1 =  200.0 + (float) (drand48 () * (w - 200.0));
  font_y1 =  100.0 + (float) (drand48 () * (h - 100.0));
  font_x2 =  200.0 + (float) (drand48 () * (w - 200.0));
  font_y2 =  100.0 + (float) (drand48 () * (h - 100.0));
}

static void
text_path_render_text1 (void)
{
  cairo_save (cr);
  cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, font_alpha1);

  cairo_set_font_size (cr, font_scale1);
  
  cairo_move_to (cr, font_x1 - font_scale1, font_y1);
  cairo_rotate(cr, font_rotate1);
  cairo_text_path (cr, "CAIRO");
  cairo_fill_preserve (cr);
  cairo_set_source_rgba (cr, 0.0, 1.0, 0.0, font_alpha1);
  cairo_stroke (cr);
  cairo_restore (cr);
}

static void
text_path_render_text2 (void)
{
  cairo_save (cr);
  cairo_set_source_rgba (cr, 0.0, 0.0, 1.0, font_alpha2);
  cairo_set_font_size (cr, font_scale2);
  cairo_move_to (cr, font_x2 - font_scale2, font_y2);
  cairo_rotate(cr, font_rotate2);
  cairo_text_path (cr, "GEARS");
  cairo_fill_preserve (cr);
  cairo_set_source_rgba (cr, 0.0, 1.0, 0.0, font_alpha2);
  cairo_stroke (cr);
  cairo_restore (cr);
}

void
text_render (int w, int h)
{
  double scale_x, scale_y;
  
  cairo_save (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_move_to (cr, 0, 0);

  scale_x = ((double) w) / (double) bg_width;
  scale_y = ((double) h) / (double) bg_height;

  cairo_scale (cr, scale_x, scale_y);

  cairo_set_source_surface (cr, bg_surface, 0.0, 0.0);
  cairo_paint (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  if (font_scale1 < font_scale2) {
    text_path_render_text1 ();
    text_path_render_text2 ();
  } else {
    text_path_render_text2 ();
    text_path_render_text1 ();
  }
  
  if (font_scale1 > 250) {
    font_scale1 = (float) (drand48 () * 50.0);
    font_x1 =  ((float) (drand48 () * (w - 400.0))) + 200.0;
    font_y1 =  ((float) (drand48 () * (h - 200.0))) + 100.0;
    font_rotate1 = (float) (drand48 () * 3.14);
    font_speed1 = (float) (drand48 () * 10.0);
    font_alpha1 = 0;
  }
  
  if (font_scale2 > 250) {
    font_scale2 = (float) (drand48 () * 50.0);
    font_x2 =  ((float) (drand48 () * (w - 400.0))) + 200.0;
    font_y2 =  ((float) (drand48 () * (h - 200.0))) + 100.0;
    font_rotate2 = (float) (drand48 () * 3.14);
    font_speed2 = (float) (drand48 () * 10.0);
    font_alpha2 = 0;
  }

  font_scale1 > 150? font_scale1 += 2: font_scale1++;
  font_scale2 > 150? font_scale2 += 2: font_scale2++;
  font_scale1 < 175?  (font_alpha1 += 0.01): (font_alpha1 -= 0.05);
  font_scale2 < 175?  (font_alpha2 += 0.01): (font_alpha2 -= 0.05);
  font_scale1 += font_speed1;
  font_scale2 += font_speed2;

  cairo_restore (cr);
}
