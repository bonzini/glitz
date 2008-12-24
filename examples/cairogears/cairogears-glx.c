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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <glitz-glx.h>
#include <cairo-xlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <strings.h>

static Display *dpy;
static Window win;
static Pixmap pixmap = None;
static XWMHints xwmh = {
    (InputHint|StateHint),
    False,
    NormalState,
    0,
    0,
    0, 0,
    0,
    0,
};

static unsigned char *image = NULL;
static cairo_surface_t *surface = NULL;
static int output_type = IMAGE_TYPE;
static int test_type = 0;
static int aa = -1;

void
resize_image (int w, int h)
{
    if (surface)
	cairo_surface_destroy (surface);
  
    if (image)
	free (image);
  
    image = (unsigned char *) malloc (w * h * 4);

    surface = cairo_image_surface_create_for_data (image, 
						   CAIRO_FORMAT_ARGB32,
						   w, h, w * 4);
}

#ifdef CAIRO_HAS_XLIB_SURFACE
void
resize_pixmap (int w, int h)
{
    if (surface)
	cairo_surface_destroy (surface);
  
    if (pixmap)
	XFreePixmap (dpy, pixmap);

    pixmap = XCreatePixmap (dpy, DefaultRootWindow (dpy), w, h,
			    DefaultDepth (dpy, DefaultScreen (dpy)));
  
    surface = cairo_xlib_surface_create (dpy,
					 pixmap,
					 DefaultVisual (dpy, DefaultScreen (dpy)),
					 w, h);
}
#endif

int
main (int argc, char **argv) {
  
    XEvent event;
    XSizeHints xsh;
    XSetWindowAttributes xswa;
    XVisualInfo *vinfo;

    glitz_drawable_format_t templ;
    glitz_drawable_format_t *dformat;
    unsigned long mask = 0;
    
    unsigned int width, height, window_width, window_height;
    int i;

    program_name = argv[0];
    
    for (i = 1; i < argc; i++) {
	if (!strcasecmp ("-image", argv[i]))
	    output_type = IMAGE_TYPE;

#ifdef CAIRO_HAS_XLIB_SURFACE
	else if (!strcasecmp ("-xrender", argv[i])) {
	    output_type = XRENDER_TYPE;
	} 
#endif

	else if (!strcasecmp ("-glx", argv[i])) {
	    output_type = GLX_TYPE;
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

    if (output_type != GLX_TYPE && test_type >= OPENGL_TYPE) {
	printf ("Sorry, this test only works with OpenGL!\n");
	usage();
	exit(1);
    }

    window_width = width = WINDOW_WIDTH;
    window_height = height = WINDOW_HEIGHT;
    
    if (aa == 3)
	templ.samples = 4;
    else
	templ.samples = 1;
    
    templ.depth_size = 16;
    if (test_type == CUBE_TYPE)
	mask |= GLITZ_FORMAT_DEPTH_SIZE_MASK;

    mask |= GLITZ_FORMAT_SAMPLES_MASK;
    
    if ((dpy = XOpenDisplay (NULL)) == NULL) {
	fprintf(stderr, "%s: can't open display: %s\n", argv[0],
		XDisplayName (NULL));
	exit(1);
    }

    if (output_type != GLX_TYPE) {
	xsh.flags = PSize;
	xsh.width = width;
	xsh.height = height;
	xsh.x = 0;
	xsh.y = 0;
	
	win = XCreateWindow (dpy, RootWindow (dpy, DefaultScreen (dpy)), 
			     xsh.x, xsh.y, xsh.width, xsh.height,
			     0, CopyFromParent, CopyFromParent,
			     CopyFromParent, 0, &xswa);
  
	XSetStandardProperties (dpy, win, PACKAGE, PACKAGE, None,
				argv, argc, &xsh);
	XSetWMHints (dpy, win, &xwmh);

	XSelectInput (dpy, win, StructureNotifyMask);
	
    } else {

	xsh.flags = PSize;
	xsh.width = width;
	xsh.height = height;
	xsh.x = 0;
	xsh.y = 0;

	mask = 0;
	
	templ.doublebuffer = 1;
	mask |= GLITZ_FORMAT_DOUBLEBUFFER_MASK;
	
	dformat = glitz_glx_find_window_format (dpy, DefaultScreen (dpy),
						mask, &templ, 0);
	
	vinfo = glitz_glx_get_visual_info_from_format (dpy,
						       DefaultScreen (dpy),
						       dformat);
	xswa.colormap =
	    XCreateColormap (dpy,
			     RootWindow (dpy, DefaultScreen (dpy)), 
			     vinfo->visual, AllocNone);
	win = XCreateWindow (dpy, RootWindow (dpy, DefaultScreen (dpy)), 
			     xsh.x, xsh.y, xsh.width, xsh.height,
			     0, vinfo->depth, CopyFromParent,
			     vinfo->visual, CWColormap, &xswa);
  
	XSetStandardProperties (dpy, win, PACKAGE, PACKAGE, None,
				argv, argc, &xsh);
	XSetWMHints (dpy, win, &xwmh);

	XSelectInput (dpy, win, StructureNotifyMask);
    }
	
    switch (output_type) {
      
    case XRENDER_TYPE:
	resize_pixmap (width, height);
	break;
      
    case IMAGE_TYPE:
	resize_image (width, height);
	break;
    case GLX_TYPE:
	drawable =
	    glitz_glx_create_drawable_for_window (dpy, 0, dformat, win,
						  width, height);
	if (!drawable) {
	    printf ("failed to create glitz drawable\n");
	    exit (1);
	}
	break;
    }
  
    if (aa == 3 && dformat->samples < 2) {
	fprintf (stderr, "hardware multi-sampling not available\n");
	exit (1);
    }

    if (drawable) {
	surface = resize_glitz_drawable (drawable, dformat, width, height);
    }

    cr = cairo_create (surface);
    cairo_set_tolerance (cr, 0.5);

    setup (test_type);

    XMapWindow (dpy, win);

    for (;;) {
	if (XPending (dpy)) {
	    XNextEvent (dpy, &event);
	    if (event.type == ConfigureNotify) {
		width = event.xconfigure.width;
		height = event.xconfigure.height;
                
		switch (output_type) {
		    
#ifdef CAIRO_HAS_XLIB_SURFACE
		case XRENDER_TYPE:
		    resize_pixmap (width, height);
		    cairo_destroy (cr);
		    cr = cairo_create (surface);
		    cairo_set_tolerance (cr, 0.5);
		    break;
#endif
		    
		case IMAGE_TYPE:
		    resize_image (width, height);
		    cairo_destroy (cr);
		    cr = cairo_create (surface);
		    cairo_set_tolerance (cr, 0.5);
		    break;
		case GLX_TYPE:
		    cairo_surface_destroy (surface);
		    surface = resize_glitz_drawable (drawable, dformat, 
						     width, height);
		    cairo_destroy (cr);
		    cr = cairo_create (surface);
		    cairo_set_tolerance (cr, 0.5);
		    break;
		}
	    }
	} else {
	    render (test_type, output_type == GLX_TYPE);
       	    switch (output_type) {
		
#ifdef CAIRO_HAS_XLIB_SURFACE
	    case XRENDER_TYPE:
		XSetWindowBackgroundPixmap (dpy, win, pixmap);
		XClearWindow (dpy, win);
		break;
#endif
		
	    case IMAGE_TYPE: {
		GC gc;
		XImage *xim;

		pixmap = XCreatePixmap (dpy, DefaultRootWindow (dpy),
					width, height,
					DefaultDepth (dpy,
						      DefaultScreen (dpy)));
		xim = XCreateImage(dpy, DefaultVisual (dpy,
						       DefaultScreen (dpy)),
				   DefaultDepth(dpy, DefaultScreen (dpy)),
				   ZPixmap, 0, (char *) image,
				   width, height, 32, 0);
		gc = XCreateGC (dpy, pixmap, 0, NULL);
		XPutImage (dpy, pixmap, gc, xim, 0, 0, 0, 0, width, height);
                    
		XFreeGC (dpy, gc);
		xim->data = NULL;
		XDestroyImage (xim);

		XSetWindowBackgroundPixmap (dpy, win, pixmap);
		XClearWindow (dpy, win);
		XFreePixmap (dpy, pixmap);
	    } break;
	    }

	    XSync (dpy, 0);
	}
    }

    exit (1);
}
