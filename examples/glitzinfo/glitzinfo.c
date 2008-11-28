/*
 * Copyright © 2004 David Reveman
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
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
 * Author: David Reveman <davidr@novell.com>
 */

#include <stdio.h>

#ifdef GLITZ_GLX_BACKEND
#include <X11/Xlib.h>
#include <glitz-glx.h>
#endif

#ifdef GLITZ_EGL_BACKEND
#include <stdlib.h>
#include <assert.h>
#include <glitz-egl.h>
#endif

#ifdef GLITZ_AGL_BACKEND
#include <glitz-agl.h>
#endif

static void
print_features (unsigned long features)
{
    printf ("direct rendering: %s\n",
	    (features & GLITZ_FEATURE_DIRECT_RENDERING_MASK)? "Yes": "No");
    printf ("texture rectangle: %s\n",
	    (features & GLITZ_FEATURE_TEXTURE_RECTANGLE_MASK)? "Yes": "No");
    printf ("texture non power of two: %s\n",
	    (features & GLITZ_FEATURE_TEXTURE_NON_POWER_OF_TWO_MASK)? "Yes":
	    "No");
    printf ("texture mirrored repeat: %s\n",
	    (features & GLITZ_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK)? "Yes":
	    "No");
    printf ("texture border clamp: %s\n",
	    (features & GLITZ_FEATURE_TEXTURE_BORDER_CLAMP_MASK)? "Yes": "No");
    printf ("multitexture: %s\n",
	    (features & GLITZ_FEATURE_MULTITEXTURE_MASK)? "Yes": "No");
    printf ("multi draw arrays: %s\n",
	    (features & GLITZ_FEATURE_MULTI_DRAW_ARRAYS_MASK)? "Yes": "No");
    printf ("texture environment combine: %s\n",
	    (features & GLITZ_FEATURE_TEXTURE_ENV_COMBINE_MASK)? "Yes":
	    "No");
    printf ("texture environment dot3: %s\n",
	    (features & GLITZ_FEATURE_TEXTURE_ENV_DOT3_MASK)? "Yes": "No");
    printf ("blend color: %s\n",
	    (features & GLITZ_FEATURE_BLEND_COLOR_MASK)? "Yes": "No");
    printf ("packed pixels: %s\n",
	    (features & GLITZ_FEATURE_PACKED_PIXELS_MASK)? "Yes": "No");
    printf ("fragment program: %s\n",
	    (features & GLITZ_FEATURE_FRAGMENT_PROGRAM_MASK)? "Yes": "No");
    printf ("vertex buffer object: %s\n",
	    (features & GLITZ_FEATURE_VERTEX_BUFFER_OBJECT_MASK)? "Yes": "No");
    printf ("pixel buffer object: %s\n",
	    (features & GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK)? "Yes": "No");
    printf ("per component rendering: %s\n",
	    (features & GLITZ_FEATURE_PER_COMPONENT_RENDERING_MASK)? "Yes":
	    "No");
    printf ("full-scene anti-aliasing: %s\n",
	    (features & GLITZ_FEATURE_MULTISAMPLE_MASK)? "Yes": "No");
    printf ("full-scene anti-aliasing filter hint: %s\n",
	    (features & GLITZ_FEATURE_MULTISAMPLE_FILTER_HINT_MASK)? "Yes":
	    "No");
    printf ("framebuffer object: %s\n",
	    (features & GLITZ_FEATURE_FRAMEBUFFER_OBJECT_MASK)? "Yes":
	    "No");
	printf ("copy sub buffer: %s\n",
	    (features & GLITZ_FEATURE_COPY_SUB_BUFFER_MASK)? "Yes":
	    "No");
}

static int
print_format (glitz_drawable_format_t *format)
{
    if (format)
    {
	printf ("0x%x\t%d/%d/%d/%d    \t%d\t%d\t%c\t%d\n",
		(int) format->id,
		format->color.red_size,
		format->color.green_size,
		format->color.blue_size,
		format->color.alpha_size,
		format->depth_size,
		format->stencil_size,
		(format->doublebuffer) ? 'y' : '.',
		format->samples);
	return 1;
    }
    return 0;
}

int
main (int argc, char **argv)
{
    glitz_drawable_format_t *dformat;
    glitz_drawable_t        *drawable;
    glitz_format_t          *formats;
    int                     i;

#ifdef GLITZ_GLX_BACKEND
    Display              *display;
    int                  screen;
    XSetWindowAttributes xswa;
    Window               win;
    XVisualInfo          *vinfo;

    if ((display = XOpenDisplay (NULL)) == NULL)
    {
	fprintf (stderr, "Error: can't open %s\n", XDisplayName (NULL));
	return 1;
    }

    screen = DefaultScreen (display);

    glitz_glx_init (NULL);
    
    dformat = glitz_glx_find_window_format (display, screen, 0, 0, 0);
    if (!dformat)
    {
	fprintf (stderr, "Error: couldn't find window format\n");
	return 1;
    }

    vinfo = glitz_glx_get_visual_info_from_format (display, screen, dformat);
    if (!vinfo) {
	fprintf (stderr, "Error: no visual info from format\n");
	return 1;
    }

    xswa.colormap = XCreateColormap (display, RootWindow (display, screen),
				     vinfo->visual, AllocNone);

    win = XCreateWindow (display, RootWindow (display, screen),
			 0, 0, 1, 1,
			 0, vinfo->depth, InputOutput,
			 vinfo->visual, CWColormap, &xswa);

    XFree (vinfo);

    drawable = glitz_glx_create_drawable_for_window (display, screen,
						     dformat, win, 1, 1);
    if (!drawable) {
	fprintf (stderr, "Error: couldn't create glitz drawable for window\n");
	return 1;
    }

    printf ("name of display: %s\n\n", DisplayString (display));
#endif

#ifdef GLITZ_EGL_BACKEND
    EGLDisplay    display;
    EGLScreenMESA screen;
    EGLModeMESA   mode;
    EGLSurface    screen_surf;
    int           maj, min, count;
    const EGLint  screenAttribs[] = {
	EGL_WIDTH, 1024,
	EGL_HEIGHT, 768,
	EGL_NONE
    };

    display = eglGetDisplay (":0");
    assert(display);

    if (!eglInitialize (display, &maj, &min))
    {
	fprintf (stderr, "Error: eglInitialize failed\n");
	return 1;
    }

    eglGetScreensMESA (display, &screen, 1, &count);
    eglGetModesMESA (display, screen, &mode, 1, &count);

    glitz_egl_init (NULL);

    dformat = glitz_egl_find_window_config (display, screen, 0, 0, 0);
    if (!dformat)
    {
	fprintf (stderr, "Error: couldn't find drawable format\n");
	return 1;
    }

    screen_surf = eglCreateScreenSurfaceMESA (display, dformat->id,
					      screenAttribs);
    if (screen_surf == EGL_NO_SURFACE)
    {
	fprintf (stderr, "Error: failed to create screen surface\n");
	return 0;
    }

    eglShowSurfaceMESA (display, screen, screen_surf, mode);

    drawable = glitz_egl_create_surface (display, screen,
					 dformat, screen_surf, 1, 1);
    if (!drawable)
    {
	fprintf (stderr, "Error: couldn't create glitz drawable for window\n");
	return 1;
    }
#endif

#ifdef GLITZ_AGL_BACKEND
    WindowRef window;
    Rect      bounds;

    glitz_agl_init ();

    dformat = glitz_agl_find_window_format (0, 0, 0);
    if (!dformat)
    {
	fprintf (stderr, "Error: couldn't find drawable format\n");
	return 1;
    }

    SetRect (&bounds, 0, 0, 1, 1);
    CreateNewWindow (kDocumentWindowClass, kWindowStandardDocumentAttributes,
		     &bounds, &window);

    drawable = glitz_agl_create_drawable_for_window (dformat, window, 1, 1);
    if (!drawable)
    {
	fprintf (stderr, "Error: couldn't create glitz drawable for window\n");
	return 1;
    }
#endif

    print_features (glitz_drawable_get_features (drawable));

    printf ("\nWindow formats:\n");
    printf ("id\tr/g/b/a    \tdepth\tstencil\tdb\tsamples\n");
    printf ("-------------------------------------"
	    "-------------------------------------\n");

    i = 0;

#ifdef GLITZ_GLX_BACKEND
    while (print_format (glitz_glx_find_window_format (display, screen,
						       0, 0, i)))
	i++;
#endif

#ifdef GLITZ_EGL_BACKEND
    while (print_format (glitz_egl_find_window_config (display, screen,
						       0, 0, i)))
	i++;
#endif

#ifdef GLITZ_AGL_BACKEND
    while (print_format (glitz_agl_find_window_format (0, 0, i)))
	i++;
#endif

    printf ("\nPbuffer formats:\n");
    printf ("id\tr/g/b/a    \tdepth\tstencil\tdb\tsamples\n");
    printf ("-------------------------------------"
	    "-------------------------------------\n");

    i = 0;
    while (print_format (glitz_find_pbuffer_format (drawable, 0, 0, i)))
	i++;

    printf ("\nOffscreen formats:\n");
    printf ("id\tr/g/b/a    \tdepth\tstencil\tdb\tsamples\n");
    printf ("-------------------------------------"
	    "-------------------------------------\n");

    i = 0;
    while (print_format (glitz_find_drawable_format (drawable, 0, 0, i)))
	i++;

    printf ("\nSurface formats:\n");
    printf ("id\ttype\tfourcc\tr/g/b/a\n");
    printf ("--------------------------------\n");

    i = 0;
    do {
	formats = glitz_find_format (drawable, 0, NULL, i++);

	if (formats) {
	    printf ("0x%x\t", (int) formats->id);

	    switch (formats->color.fourcc) {
	    case GLITZ_FOURCC_RGB:
		printf ("color\tRGB\t%d/%d/%d/%d",
			formats->color.red_size,
			formats->color.green_size,
			formats->color.blue_size,
			formats->color.alpha_size);
		break;
	    case GLITZ_FOURCC_YV12:
		printf ("color\tYV12");
		break;
	    case GLITZ_FOURCC_YUY2:
		printf ("color\tYUY2");
		break;
	    }

	    printf ("\n");
	}
    } while (formats);

    glitz_drawable_destroy (drawable);

#ifdef GLITZ_GLX_BACKEND
    glitz_glx_fini ();

    XDestroyWindow (display, win);
    XFreeColormap (display, xswa.colormap);
    XCloseDisplay (display);
#endif

#ifdef GLITZ_EGL_BACKEND
    glitz_egl_fini ();

    eglShowSurfaceMESA (display, screen, EGL_NO_SURFACE, EGL_NO_MODE_MESA);
    eglDestroySurface (display, screen_surf);
    eglTerminate (display);

#endif

#ifdef GLITZ_AGL_BACKEND
    glitz_agl_fini ();
#endif

    return 0;
}
