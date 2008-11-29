/*
 * Copyright  2004 David Reveman
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the names of
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
 * Author: David Reveman <davidr@freedesktop.org>
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "rendertest.h"
#include "glitz_common.h"

#include <glitz-egl.h>

#define EGL_DEFAULT_DST_WIDTH  1024
#define EGL_DEFAULT_DST_HEIGHT  768

static const render_backend_t _glitz_egl_render_backend = {
  _glitz_render_create_similar,
  _glitz_render_destroy,
  _glitz_render_composite,
  _glitz_render_set_pixels,
  _glitz_render_show,
  _glitz_render_set_fill,
  _glitz_render_set_component_alpha,
  _glitz_render_set_transform,
  _glitz_render_set_filter,
  _glitz_render_set_clip_rectangles,
  _glitz_render_set_clip_trapezoids
};

typedef struct egl_options {
  int samples;
  int db;
  int offscreen;
} egl_options_t;

static const render_option_t _egl_options[] = {
  { "single-buffer", 'l', NULL, 0, "    use single buffered format" },
  { "samples", 'p', "SAMPLES", 0, "   use this hardware multi-sample format" },
  { "offscreen", 'f', NULL, 0, "        use offscreen rendering" },
  { 0 }
};

static int
_parse_option (int key, char *arg, render_arg_state_t *state)
{
  egl_options_t *options = state->pointer;

  switch (key) {
  case 'l':
    options->db = 0;
    break;
  case 'p':
    options->samples = atoi (arg);
    break;
  case 'f':
    options->offscreen = 1;
    break;
  default:
    return 1;
  }

  return 0;
}

int
main (int argc, char **argv)
{
  int status, x, y;
  render_surface_t surface;
  EGLDisplay egl_display;
  EGLScreenMESA egl_screen;
  EGLModeMESA egl_mode;
  EGLSurface egl_surface;
  glitz_drawable_format_t templ;
  unsigned long mask;
  glitz_drawable_format_t *dformat;
  glitz_drawable_t *drawable, *offscreen = 0;
  render_arg_state_t state;
  egl_options_t options;
  int maj, min, count;
  EGLint screenAttribs[] = {
    EGL_WIDTH, 1024,
    EGL_HEIGHT, 768,
    EGL_NONE
  };

  options.samples = 1;
  options.db = 1;
  options.offscreen = 0;

  state.pointer = &options;

  if (render_parse_arguments (_parse_option,
			      _egl_options,
			      &state,
			      argc, argv))
    return 1;

  surface.backend = &_glitz_egl_render_backend;
  surface.flags = 0;

  x = y = 50;
  surface.width = EGL_DEFAULT_DST_WIDTH;
  surface.height = EGL_DEFAULT_DST_HEIGHT;

  egl_display = eglGetDisplay (":0");
  assert(egl_display);

  if (!eglInitialize (egl_display, &maj, &min)) {
    fprintf (stderr, "Error: eglInitialize failed\n");
    return 1;
  }

  eglGetScreensMESA (egl_display, &egl_screen, 1, &count);
  eglGetModesMESA (egl_display, egl_screen, &egl_mode, 1, &count);

  templ.samples = options.samples;
  mask = GLITZ_FORMAT_SAMPLES_MASK;

  if (options.db)
    templ.doublebuffer = 1;
  else
    templ.doublebuffer = 0;

  mask |= GLITZ_FORMAT_DOUBLEBUFFER_MASK;

  dformat = glitz_egl_find_window_config (egl_display, egl_screen,
					  mask, &templ, 0);
  if (!dformat) {
    fprintf (stderr, "Error: couldn't find window format\n");
    return 1;
  }

  egl_surface = eglCreateScreenSurfaceMESA (egl_display, dformat->id,
                                            screenAttribs);
  if (egl_surface == EGL_NO_SURFACE) {
    fprintf (stderr, "Error: failed to create screen surface\n");
    return 0;
  }

  eglShowSurfaceMESA (egl_display, egl_screen, egl_surface, egl_mode);

  drawable = glitz_egl_create_surface (egl_display, egl_screen,
                                       dformat, egl_surface,
                                       surface.width, surface.height);

  if (!drawable) {
    fprintf (stderr, "Error: couldn't create glitz drawable for window\n");
    return 1;
  }

  if (options.offscreen)
  {
      dformat = glitz_find_drawable_format (drawable, 0, 0, 0);
      if (!dformat)
      {
	  fprintf (stderr, "Error: couldn't find offscreen format\n");
	  return 1;
      }

      offscreen = glitz_create_drawable (drawable, dformat,
					 surface.width, surface.height);
      if (!offscreen)
      {
	  fprintf (stderr, "Error: couldn't create offscreen drawable\n");
	  return 1;
      }

      surface.surface =
	  _glitz_create_and_attach_surface_to_drawable (drawable,
							offscreen,
							surface.width,
							surface.height);
  }
  else
  {
    surface.surface =
      _glitz_create_and_attach_surface_to_drawable (drawable,
						    drawable,
						    surface.width,
						    surface.height);
  }

  if (!surface.surface)
    return 1;

  status = render_run (&surface, &state.settings);

  glitz_surface_destroy ((glitz_surface_t *) surface.surface);

  if (offscreen)
      glitz_drawable_destroy (offscreen);

  glitz_drawable_destroy (drawable);

  glitz_egl_fini ();

  eglDestroySurface (egl_display, egl_surface);
  eglTerminate (egl_display);

  return 0;
}
