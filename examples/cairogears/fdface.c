/*
 * $Id: fdface.c,v 1.1 2003/12/02 03:04:46 keithp Exp $
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

#include "fdface.h"

static void
draw_fancy_tick (cairo_t *cr, double radius)
{
    cairo_save (cr);
    cairo_arc (cr, 0, 0, radius, 0, 2 * M_PI);
    cairo_set_source_rgb (cr, 0.231, 0.502, 0.682);
    cairo_fill (cr);
    cairo_set_source_rgb (cr, 0.73, 0.73, 0.73);
    cairo_set_line_width (cr, radius * 2 / 3);
    cairo_arc (cr, 0, 0, radius * 2, 0, 2 * M_PI);
    cairo_stroke (cr);
    cairo_restore (cr);
}

static void
draw_plain_tick (cairo_t *cr, double radius)
{
    cairo_save (cr);
    cairo_arc (cr, 0, 0, radius, 0, 2 * M_PI);
    cairo_set_source_rgb (cr, 0.231, 0.502, 0.682);
    cairo_fill (cr);
    cairo_restore (cr);
}

void
fdface_draw (cairo_t *cr, double width, double height)
{
    int	    minute;

    cairo_save (cr);
    {
	cairo_scale (cr, width, height);
	cairo_save (cr);
	{
	    cairo_translate (cr, .15, .15);
	    cairo_scale (cr, .7, .7);
	    fdlogo_draw (cr, 1, 1);
	}
	cairo_restore (cr);
	cairo_translate (cr, 0.5, 0.5);
	cairo_scale (cr, 0.93, 0.93);
	for (minute = 0; minute < 60; minute++)
	{
	    double  degrees, radians;
	    cairo_save (cr);
	    degrees = minute * 6.0;
	    radians = degrees * M_PI / 180;
	    cairo_rotate (cr, radians);
	    cairo_translate (cr, 0, 0.5);
	    if (minute % 15 == 0)
		draw_fancy_tick (cr, 0.015);
	    else if (minute % 5 == 0)
		draw_fancy_tick (cr, 0.01);
	    else
		draw_plain_tick (cr, 0.01);
	    cairo_restore (cr);
	}
    }
    cairo_restore (cr);
}

