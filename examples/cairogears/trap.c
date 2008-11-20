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

#include <stdlib.h>

#include <cairo.h>
#include <math.h>

#define LINEWIDTH 3.0

#define FILL_R 0.1
#define FILL_G 0.1
#define FILL_B 0.75
#define FILL_OPACITY 0.5

#define STROKE_R 0.1
#define STROKE_G 0.75
#define STROKE_B 0.1
#define STROKE_OPACITY 1.0

#define NUMPTS 6

double animpts[NUMPTS * 2];
double deltas[NUMPTS * 2];

cairo_t *cr;

static void
gear (double inner_radius,
      double outer_radius,
      int teeth,
      double tooth_depth)
{
   int i;
   double r0, r1, r2;
   double angle, da;

   r0 = inner_radius;
   r1 = outer_radius - tooth_depth / 2.0;
   r2 = outer_radius + tooth_depth / 2.0;

   da = 2.0 * M_PI / (double) teeth / 4.0;

   cairo_new_path (cr);
   
   angle = 0.0;
   cairo_move_to (cr, r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da));
   
   for (i = 1; i <= teeth; i++) {
      angle = i * 2.0 * M_PI / (double) teeth;

      cairo_line_to (cr, r1 * cos (angle), r1 * sin (angle));
      cairo_line_to (cr, r2 * cos (angle + da), r2 * sin (angle + da));
      cairo_line_to (cr, r2 * cos (angle + 2 * da), r2 * sin (angle + 2 * da));

      if (i < teeth)
        cairo_line_to (cr, r1 * cos (angle + 3 * da),
                       r1 * sin (angle + 3 * da));
   }

   cairo_close_path (cr);

   cairo_move_to (cr, r0 * cos (angle + 3 * da), r0 * sin (angle + 3 * da));

   for (i = 1; i <= teeth; i++) {
      angle = i * 2.0 * M_PI / (double) teeth;

      cairo_line_to (cr, r0 * cos (angle), r0 * sin (angle));
   }

   cairo_close_path (cr);
}

void
trap_setup (cairo_t *_cr, int w, int h)
{
  int i;

  cr = cr;

  //cairo_scale (cr, 3.0, 1.0);
    
  for (i = 0; i < (NUMPTS * 2); i += 2) {
    animpts[i + 0] = (float) (drand48 () * w);
    animpts[i + 1] = (float) (drand48 () * h);
    deltas[i + 0] = (float) (drand48 () * 6.0 + 4.0);
    deltas[i + 1] = (float) (drand48 () * 6.0 + 4.0);
    if (animpts[i + 0] > w / 2.0) {
      deltas[i + 0] = -deltas[i + 0];
    }
    if (animpts[i + 1] > h / 2.0) {
      deltas[i + 1] = -deltas[i + 1];
    }
  }
}

static void
stroke_and_fill_animate (double *pts,
                         double *deltas,
                         int index,
                         int limit)
{
  double newpt = pts[index] + deltas[index];
  
  if (newpt <= 0) {
    newpt = -newpt;
    deltas[index] = (double) (drand48 () * 4.0 + 2.0);
  } else if (newpt >= (double) limit) {
    newpt = 2.0 * limit - newpt;
    deltas[index] = - (double) (drand48 () * 4.0 + 2.0);
  }
  pts[index] = newpt;
}

static void
stroke_and_fill_step (int w, int h)
{
  int i;
    
  for (i = 0; i < (NUMPTS * 2); i += 2) {
    stroke_and_fill_animate (animpts, deltas, i + 0, w);
    stroke_and_fill_animate (animpts, deltas, i + 1, h);
  }
}

double gear1_rotation = 0.35;
double gear2_rotation = 0.33;
double gear3_rotation = 0.50;

void
trap_render (int w, int h, int fill_gradient)
{
  double *ctrlpts = animpts;
  int len = (NUMPTS * 2);
  double prevx = ctrlpts[len - 2];
  double prevy = ctrlpts[len - 1];
  double curx = ctrlpts[0];
  double cury = ctrlpts[1];
  double midx = (curx + prevx) / 2.0;
  double midy = (cury + prevy) / 2.0;
  int i;

  cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);

  cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_rectangle (cr, 0, 0, w, h);
  cairo_fill (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  cairo_set_source_rgba (cr, 0.75, 0.75, 0.75, 1.0);
  cairo_set_line_width (cr, 1.0);

  cairo_save (cr);
  cairo_scale (cr, (double) w / 512.0, (double) h / 512.0);

  cairo_save (cr);
  cairo_translate (cr, 170.0, 330.0);
  cairo_rotate (cr, gear1_rotation);
  gear (30.0, 120.0, 20, 20.0);
  cairo_set_source_rgb (cr, 0.75, 0.75, 0.75);
  cairo_fill_preserve (cr);
  cairo_set_source_rgb (cr, 0.25, 0.25, 0.25);
  cairo_stroke (cr);
  cairo_restore (cr);

  cairo_save (cr);
  cairo_translate (cr, 369.0, 330.0);
  cairo_rotate (cr, gear2_rotation);
  gear (15.0, 75.0, 12, 20.0);
  cairo_set_source_rgb (cr, 0.75, 0.75, 0.75);
  cairo_fill_preserve (cr);
  cairo_set_source_rgb (cr, 0.25, 0.25, 0.25);
  cairo_stroke (cr);
  cairo_restore (cr);
  
  cairo_save (cr);
  cairo_translate (cr, 170.0, 116.0);
  cairo_rotate (cr, gear3_rotation);
  gear (20.0, 90.0, 14, 20.0);
  cairo_set_source_rgb (cr, 0.75, 0.75, 0.75);
  cairo_fill_preserve (cr);
  cairo_set_source_rgb (cr, 0.25, 0.25, 0.25);
  cairo_stroke (cr);
  cairo_restore (cr);

  cairo_restore (cr);

  gear1_rotation += 0.01;
  gear2_rotation -= (0.01 * (20.0 / 12.0));
  gear3_rotation -= (0.01 * (20.0 / 14.0));

  stroke_and_fill_step (w, h);
  
  cairo_new_path (cr);
  cairo_move_to (cr, midx, midy);
    
  for (i = 2; i <= (NUMPTS * 2); i += 2) {
    double x2, x1 = (midx + curx) / 2.0;
    double y2, y1 = (midy + cury) / 2.0;
        
    prevx = curx;
    prevy = cury;
    if (i < (NUMPTS * 2)) {
      curx = ctrlpts[i + 0];
      cury = ctrlpts[i + 1];
    } else {
      curx = ctrlpts[0];
      cury = ctrlpts[1];
    }
    midx = (curx + prevx) / 2.0;
    midy = (cury + prevy) / 2.0;
    x2 = (prevx + midx) / 2.0;
    y2 = (prevy + midy) / 2.0;
    cairo_curve_to (cr, x1, y1, x2, y2, midx, midy);
  }
  cairo_close_path (cr);

  if (fill_gradient) {
    double x1, y1, x2, y2;
    cairo_pattern_t *pattern;

    cairo_fill_extents (cr, &x1, &y1, &x2, &y2);

    pattern = cairo_pattern_create_linear (x1, y1, x2, y2);
    cairo_pattern_add_color_stop_rgb (pattern, 0.0, 1.0, 0.0, 0.0);
    cairo_pattern_add_color_stop_rgb (pattern, 1.0, 0.0, 0.0, 1.0);
    cairo_pattern_set_filter (pattern, CAIRO_FILTER_BILINEAR);

    cairo_move_to (cr, 0, 0);
    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);
  } else {
    cairo_set_source_rgba (cr, FILL_R, FILL_G, FILL_B, FILL_OPACITY);
  }
  
  cairo_fill_preserve (cr);
  cairo_set_source_rgba (cr, STROKE_R, STROKE_G, STROKE_B, STROKE_OPACITY);
  cairo_set_line_width (cr, LINEWIDTH);
  cairo_stroke (cr);
}
