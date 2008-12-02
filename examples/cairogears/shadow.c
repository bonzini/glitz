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
#include "fdface.h"
#include "fdhand.h"

cairo_surface_t *clock_bg_surface;
cairo_surface_t *clock_surface;
cairo_surface_t *glider_surface;
cairo_surface_t *fakewin_surface;
cairo_surface_t *bg_surface;
int bg_width, bg_height;

#define CLOCK_W 256
#define CLOCK_H 245

int fakewin_width;
int fakewin_height;

int glider_width;
int glider_height;

double glider_angle = 0;
double glider_pos_x = 80;
double glider_pos_y = 50;
double glider_dir_x = 1;
double glider_dir_y = 1;

cairo_t *cr;

static void
render_fakewin (void)
{
    cairo_t *cr2;

    cr2 = cairo_create (fakewin_surface);
    cairo_set_operator (cr2, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb (cr2, 1.0, 1.0, 1.0);
    cairo_move_to (cr2, 0, 0);
    cairo_rectangle (cr2, 17, 34, fakewin_width - 32, fakewin_height - 50);
    cairo_fill (cr2);
    cairo_set_operator (cr2, CAIRO_OPERATOR_OVER);
    cairo_translate (cr2, (int) glider_pos_x, (int) glider_pos_y);

    cairo_set_source_surface (cr2, glider_surface, 0.0, 0.0);
    cairo_paint (cr2);
    cairo_destroy (cr2);

    glider_pos_x += glider_dir_x;
    glider_pos_y += glider_dir_y;

    if (glider_pos_x <= 18)
	glider_dir_x = -glider_dir_x;
    
    if (glider_pos_y <= 35)
	glider_dir_y = -glider_dir_y;
    
    if (glider_pos_x >= (fakewin_width - 16 - 37))
	glider_dir_x = -glider_dir_x;
    
    if (glider_pos_y >= (fakewin_height - 19 - 37))
	glider_dir_y = -glider_dir_y;
}

static void
render_clock (void)
{
    cairo_t *cr2;

    cr2 = cairo_create (clock_surface);
    cairo_set_operator (cr2, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba (cr2, 0.0, 0.0, 0.0, 0.0);
    cairo_rectangle (cr2, 0, 0, CLOCK_W, CLOCK_H);
    cairo_fill (cr2);
    cairo_set_operator (cr2, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgb (cr2, 0.0, 0.0, 0.0);
    cairo_move_to (cr2, 0, 0);
    
    cairo_set_source_surface (cr2, clock_bg_surface, 0.0, 0.0);
    cairo_paint (cr2);
    
    fdhand_draw_now (cr2, CLOCK_W, CLOCK_H, 1);
    cairo_destroy (cr2);
}

void
shadow_setup (cairo_t *_cr, int w, int h)
{
  clock_surface = cairo_surface_create_similar (cairo_get_target (_cr),
						CAIRO_CONTENT_COLOR_ALPHA,
						CLOCK_W, CLOCK_H);
  
  clock_bg_surface = cairo_surface_create_similar (cairo_get_target (_cr),
						   CAIRO_CONTENT_COLOR_ALPHA,
						   CLOCK_W, CLOCK_H);


  bg_surface = cairo_glitz_surface_create_from_png (_cr, "desktop.png",
						    &bg_width, &bg_height);
  if (cairo_surface_status (bg_surface)) {
    printf ("error reading desktop.png\n");
    exit(1);
  }

  fakewin_surface = cairo_glitz_surface_create_from_png (_cr, "fakewin.png",
							 &fakewin_width,
							 &fakewin_height);
  if (cairo_surface_status (fakewin_surface)) {
    printf ("error reading fakewin.png\n");
    exit(1);
  }
  
  glider_surface = cairo_glitz_surface_create_from_png (_cr, "glider.png",
							&glider_width,
							&glider_height);
  if (cairo_surface_status (glider_surface)) {
    printf ("error reading glider.png\n");
    exit(1);
  }

  cr = cairo_create (clock_bg_surface);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
  cairo_rectangle (cr, 0, 0, CLOCK_W, CLOCK_H);
  cairo_fill (cr);

  cairo_save (cr);
  cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.25);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  cairo_scale (cr, CLOCK_W, CLOCK_H);
  cairo_translate (cr, 0.5, 0.5);
  cairo_arc (cr, 0, 0, 0.5, 0, 2 * M_PI);
  cairo_fill (cr);
  cairo_restore (cr);
  
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  fdface_draw (cr, CLOCK_W, CLOCK_H);
  cairo_destroy (cr);

  render_clock ();
  render_fakewin ();

  cr = _cr;
}

long int shadow_cnt = 0;

double fw_x = 250.0, fw_y = 20.0;
double cl_x = 50.0, cl_y = 180.0;
double dst_x = 20.0, dst_y = 270.0;
double start_x = 250.0, start_y = 20.0;

int which = 1;

static int
shadow_move_towards_point (double *current_x,
                           double *current_y)
{
  double to_go_x = dst_x - *current_x;
  double to_go_y = dst_y - *current_y;
  double done_x = *current_x - start_x;
  double done_y = *current_y - start_y;
  
  double dist_from_start = fabs (to_go_x) + fabs (to_go_y);
  double dist_from_goal = fabs (done_x) + fabs (done_y);
  double speed = (dist_from_start < dist_from_goal) ?
    dist_from_start: dist_from_goal;
  double speed_div;
  double angle;
  
  speed *= 10.0;
  
  speed_div = fabs (dst_x - start_x) + fabs (dst_y - start_y);
  
  speed /= (speed_div > 1.0)? speed_div: 1.0;
  speed += 0.1;
  
  angle = fabs (atan (to_go_y / to_go_x));

  *current_x += speed * cos (angle) * ((dst_x > start_x)? 1.0: -1.0);
  *current_y += speed * sin (angle) * ((dst_y > start_y)? 1.0: -1.0);

  if (start_x <= dst_x) {
    if (*current_x >= dst_x)
      return 1;
  } else {
    if (*current_x <= dst_x)
      return 1;
  }

  if (start_y <= dst_y) {
    if (*current_y >= dst_y)
      return 1;
  } else {
    if (*current_y <= dst_y)
      return 1;
  }
  
  return 0;
}

double shadow_alpha = 0.4;
double max_shadow_offset = 60.0;

void
shadow_render (int w, int h)
{
  double scale_x, scale_y;
  int move_done;
  double shadow_offset_x, shadow_offset_y;
  double light_x, light_y;

  light_x = w / 2.0;
  light_y = h / 2.0;
  
  cairo_save (cr);

  cairo_save (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  
  scale_x = ((double) w) / (double) bg_width;
  scale_y = ((double) h) / (double) bg_height;

  cairo_scale (cr, scale_x, scale_y);
  cairo_set_source_surface (cr, bg_surface, 0.0, 0.0);
  cairo_paint (cr);
  cairo_restore (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  if (!(shadow_cnt % 10))
    render_clock ();
  else
    if (!(shadow_cnt % 5))
      render_fakewin ();
  
  shadow_cnt++;

  if (shadow_cnt >= 1000000)
    shadow_cnt = 0;  

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  shadow_offset_x = max_shadow_offset *
    ((fw_x + fakewin_width / 2) - light_x) / (w / 2.0);
  shadow_offset_y = max_shadow_offset *
    ((fw_y + fakewin_height / 2) - light_y) / (h / 2.0);
  
  cairo_save (cr);
  cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, shadow_alpha);
  cairo_mask_surface (cr, fakewin_surface,
		      fw_x + shadow_offset_x - 0.05 * fakewin_width,
		      fw_y + shadow_offset_y - 0.05 * fakewin_height);
  cairo_paint (cr);
  cairo_restore (cr);
  
  cairo_save (cr);
  cairo_translate (cr, (int) fw_x, (int) fw_y);
  cairo_set_source_surface (cr, fakewin_surface, 0.0, 0.0);
  cairo_paint (cr);
  cairo_restore (cr);

  shadow_offset_x = max_shadow_offset *
    ((cl_x + CLOCK_W / 2) - light_x) / (w / 2.0);
  shadow_offset_y = max_shadow_offset *
    ((cl_y + CLOCK_H / 2) - light_y) / (h / 2.0);
  
  cairo_save (cr);
  cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, shadow_alpha);
  cairo_mask_surface (cr, clock_surface,
		      cl_x + shadow_offset_x - 0.05 * CLOCK_W,
		      cl_y + shadow_offset_y - 0.05 * CLOCK_H);
  cairo_paint (cr);
  cairo_restore (cr);

  cairo_save (cr);
  cairo_translate (cr, (int) cl_x, (int) cl_y);
  cairo_set_source_surface (cr, clock_surface, 0.0, 0.0);
  cairo_paint (cr);
  cairo_restore (cr);

  if (which) {
    move_done = shadow_move_towards_point (&fw_x, &fw_y);
  } else {
    move_done = shadow_move_towards_point (&cl_x, &cl_y);
  }

  if (move_done) {
    which = (int) (drand48 () + 0.5);
    
    dst_x = drand48 () *
      (w - ((which)? fakewin_width : CLOCK_W) -
       max_shadow_offset);
    dst_y = drand48 () *
      (h - ((which)? fakewin_height: CLOCK_H) -
       max_shadow_offset);

    if (which) {
      start_x = fw_x;
      start_y = fw_y;
    } else {
      start_x = cl_x;
      start_y = cl_y;
    }
  }
  
  cairo_restore (cr);
}
