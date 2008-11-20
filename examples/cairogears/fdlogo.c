/*
 * $Id: fdlogo.c,v 1.1 2003/12/02 03:04:47 keithp Exp $
 *
 * Copyright © 2003 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "fdlogo.h"

static void
draw_boundary (cairo_t *cr)
{
    cairo_move_to      (cr,  63.000,  36.000);
    
    cairo_curve_to     (cr,  63.000,  43.000,
			     58.000,  47.000,
			     51.000,  47.000);

    cairo_line_to      (cr,   13.000,  47.000);
    
    cairo_curve_to     (cr,    6.000,  47.000,
			       1.000,  43.000,
			       1.000,  36.000);
			   
    cairo_line_to      (cr,    1.000,  12.000);
    
    cairo_curve_to     (cr,    1.000,  5.000,
			       6.000,  1.000,
			      13.000,  1.000);
    
    cairo_line_to      (cr,   51.000,  1.000);
    
    cairo_curve_to     (cr,   58.000,  1.000,
			      63.000,  5.000,
			      63.000,  12.000);
    cairo_close_path (cr);
}

static void
draw_outline (cairo_t *cr)
{
    /* cairo_set_rgb_color (cr, 0.73, 0.73, 0.73); */
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    cairo_set_line_width (cr, 2);
    draw_boundary (cr);
    cairo_stroke (cr);
}

static void
draw_background (cairo_t *cr)
{
    cairo_save (cr);
    cairo_set_source_rgb (cr, 0.231, 0.502, 0.682);
    cairo_translate (cr, 3.5, 3.5);
    cairo_scale (cr, 0.887, 0.848);
    draw_boundary (cr);
    cairo_fill (cr);
    cairo_restore (cr);
}

static void
draw_window (cairo_t *cr)
{
    cairo_move_to (cr,  -6.00, -7.125);
    
    cairo_line_to (cr,   6.00, -7.125);
    
    cairo_curve_to (cr,  8.00, -7.125,
			 9.00, -6.125,
			 9.00, -4.125);
    
    cairo_line_to (cr,   9.00,  4.125);

    cairo_curve_to (cr,  9.00,  6.125,
			 8.00,  7.125,
			 6.00,  7.125);

    cairo_line_to (cr,  -6.00,  7.125);

    cairo_curve_to (cr, -8.00,  7.125,
			-9.00,  6.125,
			-9.00,  4.125);

    cairo_line_to (cr,	-9.00, -4.125);

    cairo_curve_to (cr,	-9.00, -6.125,
			-8.00, -7.125,
			-6.00, -7.125);
    cairo_close_path (cr);
}

static void
draw_window_at (cairo_t *cr, double x, double y, double scale)
{
    cairo_save (cr);
    {
	cairo_translate (cr, x, y);
	cairo_scale (cr, scale, scale);
	draw_window (cr);
	cairo_save (cr);
	{
	    cairo_set_source_rgb (cr, 1, 1, 1);
	    cairo_fill (cr);
	}
	cairo_restore (cr);
	cairo_set_source_rgb (cr, 0.231, 0.502, 0.682);
	cairo_scale (cr, 1/scale, 1/scale);
	cairo_stroke (cr);
    }
    cairo_restore (cr);
}

void
draw_windows (cairo_t *cr)
{
    cairo_save (cr);
    {
	cairo_move_to (cr, 18.00, 16.125);
	cairo_line_to (cr, 48.25, 20.375);
	cairo_line_to (cr, 30.25, 35.825);
	cairo_close_path (cr);
	cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.5);
	cairo_stroke (cr);
    }
    cairo_restore (cr);
    draw_window_at (cr, 18.00, 16.125, 1);
    draw_window_at (cr, 48.25, 20.375, 0.8);
    draw_window_at (cr, 30.25, 35.825, 0.5);
}

#define FDLOGO_ROT_X_FACTOR	1.086
#define FDLOGO_ROT_Y_FACTOR	1.213
#define FDLOGO_WIDTH		(64 * FDLOGO_ROT_X_FACTOR)
#define FDLOGO_HEIGHT		(48 * FDLOGO_ROT_Y_FACTOR)

void
fdlogo_draw (cairo_t *cr, double width, double height)
{
    double  x_scale, y_scale, scale, x_off, y_off;
    cairo_save (cr);
    x_scale = width / FDLOGO_WIDTH;
    y_scale = height / FDLOGO_HEIGHT;
    scale = x_scale < y_scale ? x_scale : y_scale;
    x_off = (width - (scale * FDLOGO_WIDTH)) / 2;
    y_off = (height - (scale * FDLOGO_HEIGHT)) / 2;
    cairo_translate (cr, x_off, y_off);
    cairo_scale (cr, scale, scale);
    
    cairo_translate (cr, -2.5, 14.75);
    cairo_rotate (cr, -0.274990703529840);
    
    draw_outline (cr);
    draw_background (cr);
    draw_windows (cr);
    cairo_restore (cr);
}
