/*
 * Copyright © 2004 David Reveman
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

#include "rendertest.h"
#include "glitz_common.h"

#include <glitz-agl.h>

static const render_backend_t _glitz_agl_render_backend = {
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

typedef struct agl_options {
  int samples;
  int db;
  int offscreen;
} agl_options_t;

static const render_option_t _glx_options[] = {
  { "single-buffer", 'l', NULL, 0, "    use single buffered format" },
  { "samples", 'p', "SAMPLES", 0, "   use this hardware multi-sample format" },
  { "offscreen", 'f', NULL, 0, "        use offscreen rendering" },
  { 0 }
};

static int
_parse_option (int key, char *arg, render_arg_state_t *state)
{
  agl_options_t *options = state->pointer;

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
  glitz_drawable_format_t templ;
  unsigned long mask;
  glitz_drawable_format_t *dformat;
  glitz_drawable_t *drawable, *offscreen = 0;
  render_arg_state_t state;
  agl_options_t options;
  WindowRef win;
  Rect win_bounds;
  CFStringRef title;

  options.samples = 1;
  options.db = 1;
  options.offscreen = 0;

  state.pointer = &options;

  if (render_parse_arguments (_parse_option,
			      _glx_options,
			      &state,
			      argc, argv))
    return 1;

  surface.backend = &_glitz_agl_render_backend;
  surface.flags = 0;

  x = y = 50;
  surface.width = RENDER_DEFAULT_DST_WIDTH;
  surface.height = RENDER_DEFAULT_DST_HEIGHT;

  SetRect (&win_bounds, x, y, x + surface.width, y + surface.height);

  InitCursor ();

  CreateNewWindow (kDocumentWindowClass,
		   kWindowStandardDocumentAttributes,
		   &win_bounds,
		   &win);

  SetPortWindowPort (win);

  title = CFSTR (PACKAGE);
  SetWindowTitleWithCFString (win, title);
  CFRelease (title);

  SelectWindow (win);

  templ.samples = options.samples;
  mask = GLITZ_FORMAT_SAMPLES_MASK;

  if (options.db)
    templ.doublebuffer = 1;
  else
    templ.doublebuffer = 0;

  mask |= GLITZ_FORMAT_DOUBLEBUFFER_MASK;

  dformat = glitz_agl_find_window_format (mask, &templ, 0);
  if (!dformat) {
    fprintf (stderr, "Error: couldn't find window format\n");
    return 1;
  }

  drawable =
    glitz_agl_create_drawable_for_window (dformat, win,
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

  ShowWindow (win);

  status = render_run (&surface, &state.settings);

  glitz_surface_destroy ((glitz_surface_t *) surface.surface);

  if (offscreen)
      glitz_drawable_destroy (offscreen);

  glitz_drawable_destroy (drawable);

  glitz_agl_fini ();

  return status;
}
