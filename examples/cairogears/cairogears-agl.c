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

#include "cairogears.h"
#include <glitz-agl.h>
#include <OpenGL/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <strings.h>

static WindowRef window;
static cairo_surface_t *surface = NULL;
static int test_type = 0;
static int aa = -1;

int
main (int argc, char **argv)
{
    Rect window_bounds;
    CFStringRef title;
    EventRecord event;
    RgnHandle region_handler;

    glitz_drawable_format_t templ;
    glitz_drawable_format_t *dformat;
    unsigned long mask = 0;
    
    unsigned int width, height, window_width, window_height;
    int i;

    program_name = argv[0];
    
    for (i = 1; i < argc; i++) {
	if (!strcasecmp ("-agl", argv[i])) {
	    ;
	} else if (!strcasecmp ("-noaa", argv[i])) {
	    aa = 0;
	} else if (!strcasecmp ("-swaa", argv[i])) {
	    aa = 1;
	} else if (!strcasecmp ("-hwaa", argv[i])) {
	    aa = 3;
	} else {
	    test_type = get_test_type (argv[i]);
	}
    }
  
    if (!test_type) {
	usage();
	exit(1);
    }

    window_width = width = WINDOW_WIDTH;
    window_height = height = WINDOW_HEIGHT;
    
    if (aa == 3)
	templ.samples = 4;
    else
	templ.samples = 1;
    
    mask |= GLITZ_FORMAT_SAMPLES_MASK;
    
	SetRect (&window_bounds, 100, 100, 100 + WINDOW_WIDTH,
		 100 + WINDOW_HEIGHT);
	
	InitCursor();
	
	CreateNewWindow (kDocumentWindowClass,
			 kWindowStandardDocumentAttributes,
			 &window_bounds,
			 &window);

	SetPortWindowPort (window);
	
	title = CFSTR (PACKAGE);
	SetWindowTitleWithCFString (window, title);
	CFRelease (title);
	
	SelectWindow (window);
	
	dformat = glitz_agl_find_window_format (mask, &templ, 0);
	if (!dformat) {
	    fprintf (stderr, "Error: couldn't find window format\n");
	    exit (1);
	}
  
	drawable = glitz_agl_create_drawable_for_window (dformat, window,
							 width, height);
	if (!drawable) {
	    printf ("failed to create glitz drawable\n");
	    exit (1);
	}

    if (aa == 3 && dformat->samples < 2) {
	fprintf (stderr, "hardware multi-sampling not available\n");
	exit (1);
    }

	surface = resize_glitz_drawable (drawable, dformat, width, height);

    cr = cairo_create (surface);
    cairo_set_tolerance (cr, 0.5);

    setup (test_type, width, height);

    ShowWindow (window);

    region_handler = NewRgn ();
    
    for (;;) {
	if (WaitNextEvent (everyEvent, &event, 0, region_handler)) {
	    WindowPartCode part_code = FindWindow (event.where, &window);
	    
	    switch (event.what) {
	    case mouseDown:
		if (window != FrontWindow ())
		    SelectWindow (window);
		    
		switch (part_code) {
		case inDrag:
		    DragWindow (window, event.where, NULL);
		    break;
		}
		break;
	    }
	} else {
	    render (test_type, TRUE, width, height);
	}
    }
  
    exit (1);
}
