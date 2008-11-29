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

#include "rendertest.h"
#include "images.h"
#include "rects.h"
#include "traps.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>

typedef struct render_info {
  render_surface_t *surface;
  render_surface_t *bg;
  render_surface_t *logo;
  render_settings_t *s;
  int w, h, success, not_supported, failed;
} render_info_t;

static char _success[] = {
  27, '[', '1', ';', '3', '2', 'm', /* green */
  's', 'u', 'c', 'c', 'e', 's', 's',
  27, '[', '0', 'm', '\0'
};

static char _not_supported[] = {
  27, '[', '1', ';', '3', '3', 'm', /* yellow */
  'n', 'o', 't', ' ', 's', 'u', 'p', 'p', 'o', 'r', 't', 'e', 'd',
  27, '[', '0', 'm', '\0'
};

static char _failed[] = {
  27, '[', '1', ';', '3', '1', 'm', /* red */
  'f', 'a', 'i', 'l', 'e', 'd',
  27, '[', '0', 'm', '\0'
};

static char _no_memory[] = {
  27, '[', '1', ';', '3', '1', 'm', /* red */
  'n', 'o', ' ', 'm', 'e', 'm', 'o', 'r', 'y',
  27, '[', '0', 'm', '\0'
};


static char *
_render_status_string (render_status_t status)
{
  switch (status) {
  case RENDER_STATUS_SUCCESS:
    return _success;
  case RENDER_STATUS_NOT_SUPPORTED:
    return _not_supported;
  case RENDER_STATUS_NO_MEMORY:
    return _no_memory;
  case RENDER_STATUS_FAILED:
  default:
    return _failed;
  }
}

static char *
_render_fill_string (render_fill_t fill)
{
  switch (fill) {
  case RENDER_FILL_TRANSPARENT:
    return "(transparent)";
  case RENDER_FILL_NEAREST:
    return "(nearest)    ";
  case RENDER_FILL_REPEAT:
    return "(repeat)     ";
  case RENDER_FILL_REFLECT:
    return "(reflect)    ";
  case RENDER_FILL_NONE:
  default:
    return "             ";
  }
}

static char *
_render_format_string (render_format_t format)
{
  switch (format) {
  case RENDER_FORMAT_RGB24:
    return "RGB24";
  case RENDER_FORMAT_ARGB32:
    return "ARGB32";
  case RENDER_FORMAT_YV12:
    return "YV12";
  case RENDER_FORMAT_YUY2:
    return "YUY2";
  default:
    return "?";
  }
}

static char *
_render_operator_string (render_operator_t op)
{
  switch (op) {
  case RENDER_OPERATOR_CLEAR:
    return "CLEAR";
  case RENDER_OPERATOR_SRC:
    return "SRC";
  case RENDER_OPERATOR_DST:
    return "DST";
  case RENDER_OPERATOR_OVER_REVERSE:
    return "OVER_REVERSE";
  case RENDER_OPERATOR_IN:
    return "IN";
  case RENDER_OPERATOR_IN_REVERSE:
    return "IN_REVERSE";
  case RENDER_OPERATOR_OUT:
    return "OUT";
  case RENDER_OPERATOR_OUT_REVERSE:
    return "OUT_REVERSE";
  case RENDER_OPERATOR_ATOP:
    return "ATOP";
  case RENDER_OPERATOR_ATOP_REVERSE:
    return "ATOP_REVERSE";
  case RENDER_OPERATOR_XOR:
    return "XOR";
  case RENDER_OPERATOR_ADD:
    return "ADD";
  default:
    return "OVER";
  }
}

static render_status_t
_render_set_background (render_surface_t *surface,
			render_surface_t *grad,
			render_surface_t *logo)
{
  render_status_t status;

  status =
    surface->backend->composite (RENDER_OPERATOR_SRC,
				 grad,
				 NULL,
				 surface,
				 0, 0,
				 0, 0,
				 0, 0,
				 grad->width,
				 grad->height);
  if (status)
    return status;

  status =
    surface->backend->composite (RENDER_OPERATOR_OVER,
				 logo,
				 NULL,
				 surface,
				 0, 0,
				 0, 0,
				 surface->width / 2 - logo->width / 2,
				 surface->height / 2 - logo->height / 2,
				 logo->width,
				 logo->height);

  return status;
}

static void
_print_info (char *text,
	     int test,
	     render_settings_t *s)
{
  if (!s->quiet) {
    printf ("\n%d. ", test);
    printf (text);
    fflush (stdout);
  }
}

static void
_print_result (render_status_t status,
	       int time,
	       render_info_t *info)
{
  if (status == RENDER_STATUS_SUCCESS) {
    info->success++;
  } else if (status == RENDER_STATUS_NOT_SUPPORTED)
    info->not_supported++;
  else
    info->failed++;

  if (!info->s->quiet) {
    putchar ('[');
    printf (_render_status_string (status));
    putchar (']');

    if (info->s->time && (status == RENDER_STATUS_SUCCESS))
      printf (" (%d.%.3d sec)", time / 1000, time % 1000);

    fflush (stdout);
  }

  if (info->s->interactive)
    getchar ();
  else if (!info->s->quiet)
    putchar ('\n');

  if (info->s->sleep >= 0) {
    int sec = info->s->sleep;

    while (sec > 0) {
      if (!info->s->quiet)
	printf ("\rnext test in %d second%s", sec, (sec == 1)? " ": "s");;
      sec--;
      fflush (stdout);
      sleep (1);
    }

    if (!info->s->quiet) {
      printf ("\r");
      printf ("                                  ");
      printf ("\r");
    }
  }
}

static render_surface_t *
create_surface_for_data (render_surface_t *other,
			 render_format_t format,
			 unsigned char *data,
			 unsigned int width,
			 unsigned int height)
{
  render_surface_t *surface = NULL;
  render_status_t status;

  surface = other->backend->create_similar (other, format, width, height);
  if (surface == NULL) {
    fprintf (stderr, "failed to create surface for png\n");
    return NULL;
  }

  other->backend->set_fill (surface, RENDER_FILL_NONE);

  if (width == 1 && height == 1)
    surface->flags |= RENDER_SURFACE_FLAG_SOLID_MASK;

  if (format == RENDER_FORMAT_YV12 || format == RENDER_FORMAT_YUY2)
      format = RENDER_FORMAT_RGB24;

  status = other->backend->set_pixels (surface, format, data);
  if (status) {
    fprintf (stderr, "put_pixels: %s\n", _render_status_string (status));
    other->backend->destroy (surface);
    surface = NULL;
  }

  return surface;
}

static render_surface_t *
create_surface_for_png (render_surface_t *other,
			render_format_t format,
			unsigned char *png_data)
{
  render_surface_t *surface = NULL;
  unsigned int w, h;
  unsigned char *data;

  if (render_read_png (png_data, &w, &h, &data)) {
    fprintf (stderr, "failed to read png data\n");
    return NULL;
  }

  surface = create_surface_for_data (other, format, data, w, h);

  free (data);

  return surface;
}

static render_fill_t _fill_type[] = {
  RENDER_FILL_NONE,
  RENDER_FILL_TRANSPARENT,
  RENDER_FILL_NEAREST,
  RENDER_FILL_REPEAT,
  RENDER_FILL_REFLECT
};

static void
_print_comp_info (render_surface_t *src,
		  render_surface_t *mask,
		  render_fill_t src_fill,
		  render_fill_t mask_fill,
		  render_settings_t *s)
{
  if (!s->quiet) {
    if (mask)
      printf ("  source %s IN mask %s %s dest: ",
	      _render_fill_string (src_fill),
	      _render_fill_string (mask_fill),
	      _render_operator_string (s->op));
    else
      printf ("  source %s %s dest: ",
	      _render_fill_string (src_fill),
	      _render_operator_string (s->op));

    fflush (stdout);
  }
}

static void
timeval_subtract (const struct timeval *x,
		  const struct timeval *y,
		  struct timeval *diff)
{
  if (x->tv_sec == y->tv_sec || x->tv_usec >= y->tv_usec) {
    diff->tv_sec = x->tv_sec - y->tv_sec;
    diff->tv_usec = x->tv_usec - y->tv_usec;
  } else {
    diff->tv_sec = x->tv_sec - 1 - y->tv_sec;
    diff->tv_usec = 1000000 + x->tv_usec - y->tv_usec;
  }
}

static render_fill_t
_next_fill_type (render_surface_t *surface, int *i)
{
  if (!surface) {
    *i = 4;

    return RENDER_FILL_NONE;
  }

  if (SURFACE_SOLID (surface)) {
    *i = 4;

    return _fill_type[0];
  }

  if (SURFACE_TRANSFORM (surface) || SURFACE_FILTER (surface)) {
    if (*i == 0)
      *i = 1;
  }

  return _fill_type[*i];
}

static void
_render_composite_tests (render_info_t *info,
			 render_surface_t *src,
			 render_surface_t *mask,
			 render_status_t status)
{
  render_status_t src_status, mask_status;
  render_fill_t src_fill, mask_fill;
  int i, j, k;

  for (i = 0; i < 5; i++) {
    src_fill = _next_fill_type (src, &i);

    src_status = status;

    if ((!src_status) && (!SURFACE_SOLID (src)))
      src_status = info->surface->backend->set_fill (src, src_fill);

    for (j = 0; j < 5; j++) {
      struct timeval tv1, tv2, tv_diff;

      mask_fill = _next_fill_type (mask, &j);

      mask_status = src_status;

      _print_comp_info (src, mask, src_fill, mask_fill, info->s);
      _render_set_background (info->surface, info->bg, info->logo);

      if (!mask_status) {
	if (mask && (!SURFACE_SOLID (mask)))
	  mask_status = info->surface->backend->set_fill (mask, mask_fill);

	if (!mask_status) {
	  int x_dst = info->w / 10;
	  int y_dst = info->h / 10;
	  int width = (info->w * 8) / 10;
	  int height = (info->h * 8) / 10;

	  switch (info->s->clip) {
	  case RENDER_CLIP_RECTANGLES:
	    mask_status =
	      info->surface->backend->set_clip_rectangles
	      (info->surface,
	       x_dst + (width - 256) / 2,
	       y_dst + (height - 224) / 2,
	       check_rects,
	       sizeof (check_rects) / sizeof (render_rectangle_t));
	    break;
	  case RENDER_CLIP_TRAPEZOIDS:
	    mask_status =
	      info->surface->backend->set_clip_trapezoids
	      (info->surface,
	       x_dst + (width - 210) / 2,
	       y_dst + (height - 210) / 2,
	       curve_rectangle_traps,
	       sizeof (curve_rectangle_traps) / sizeof (render_trapezoid_t));
	    break;
	  default:
	    break;
	  }

	  if (!mask_status) {
	    gettimeofday (&tv1, NULL);

	    for (k = 0; (!mask_status) && k < info->s->repeat; k++)
	      mask_status =
		info->surface->backend->composite (info->s->op,
						   src,
						   mask,
						   info->surface,
						   0, 0,
						   0, 0,
						   x_dst,
						   y_dst,
						   width,
						   height);
	  }

	  switch (info->s->clip) {
	  case RENDER_CLIP_RECTANGLES:
	    info->surface->backend->set_clip_rectangles (info->surface, 0, 0,
							 NULL, 0);
	    break;
	  case RENDER_CLIP_TRAPEZOIDS:
	    info->surface->backend->set_clip_trapezoids (info->surface, 0, 0,
							 NULL, 0);
	    break;
	  default:
	    break;
	  }
	}
      }

      info->surface->backend->show (info->surface);

      gettimeofday (&tv2, NULL);
      timeval_subtract (&tv2, &tv1, &tv_diff);

      _print_result (mask_status,
		     tv_diff.tv_sec * 1000 + tv_diff.tv_usec / 1000,
		     info);
    }
  }
}

static render_matrix_t _identity = {
  {
    { 1.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0 }
  }
};

static render_status_t
_render_set_transform (render_surface_t *surface,
		       render_matrix_t *matrix)
{
  if (memcmp (matrix, &_identity, sizeof (_identity)))
    surface->flags |= RENDER_SURFACE_FLAG_TRANSFORM_MASK;
  else
    surface->flags &= ~RENDER_SURFACE_FLAG_TRANSFORM_MASK;

  return surface->backend->set_transform (surface, matrix);
}

static render_status_t
_render_set_filter (render_surface_t *surface,
		    render_filter_t filter,
		    render_fixed16_16_t *params,
		    int n_params)
{
  render_fixed16_16_t *normalized = 0;
  render_status_t     status;

  if (filter != RENDER_FILTER_NEAREST && filter != RENDER_FILTER_BILINEAR)
    surface->flags |= RENDER_SURFACE_FLAG_FILTER_MASK;
  else
    surface->flags &= ~RENDER_SURFACE_FLAG_FILTER_MASK;

  if (filter == RENDER_FILTER_CONVOLUTION)
  {
      render_fixed16_16_t sum = 0;
      int                 i;

      for (i = 2; i < n_params; i++)
	  sum += params[i];

      if (sum != FIXED1)
      {
	  normalized = malloc (sizeof (render_fixed16_16_t) * n_params);
	  if (!normalized)
	      return RENDER_STATUS_NO_MEMORY;

	  normalized[0] = params[0];
	  normalized[1] = params[1];

	  for (i = 2; i < n_params; i++)
	      normalized[i] = params[i] / (sum >> 16);

	  params = normalized;
      }
  }

  status = surface->backend->set_filter (surface, filter, params, n_params);

  if (normalized)
      free (normalized);

  return status;
}

#define TEST_CHECK \
  (info.s->first_test <= ++test && info.s->last_test >= test)

int
render_run (render_surface_t *surface, render_settings_t *settings)
{
  unsigned char *data;
  render_info_t info;
  int x, y, w, h, test = 0;
  render_surface_t *a_mask, *argb_mask, *solid_red, *solid_blue, *glider,
    *big_glider, *grad2x1, *grad11x1;
  render_status_t status;
  int grad2x1_colors[] = { 0x7f4087d1, 0xffff6363 } ;
  int grad11x1_colors[] = {
    0xff0c5b91, 0xff85abc6, 0xfffffbfb, 0xffa08d82, 0xff421f08, 0xff516737,
    0xff60af65, 0xff378441, 0xff0f591d, 0xff156d23, 0xff1c8129
  };
  int red_color = 0x80ff0000;
  int blue_color = 0xc06060c0;

  info.w = w = surface->width;
  info.h = h = surface->height;
  info.surface = surface;
  info.s = settings;
  info.success = info.not_supported = info.failed = 0;

  info.logo = create_surface_for_png (surface, RENDER_FORMAT_ARGB32, logo_png);
  if (info.logo == NULL) {
    fprintf (stderr, "failed to create logo surface\n");
    return RENDER_STATUS_FAILED;
  }

  if (info.s->npot)
    a_mask = create_surface_for_png (surface, RENDER_FORMAT_A8,
				     a_mask_npot_png);
  else
    a_mask = create_surface_for_png (surface, RENDER_FORMAT_A8,
				     a_mask_pot_png);

  if (a_mask == NULL) {
    fprintf (stderr, "failed to create alpha mask surface\n");
    return RENDER_STATUS_FAILED;
  }

  if (info.s->npot)
    argb_mask = create_surface_for_png (surface, RENDER_FORMAT_ARGB32,
					argb_mask_npot_png);
  else
    argb_mask = create_surface_for_png (surface, RENDER_FORMAT_ARGB32,
					argb_mask_pot_png);

  if (argb_mask == NULL) {
    fprintf (stderr, "failed to create argb mask surface\n");
    return RENDER_STATUS_FAILED;
  }

  if (info.s->npot)
    glider = create_surface_for_png (surface, info.s->format,
				     glider_npot_png);
  else
    glider = create_surface_for_png (surface, info.s->format,
				     glider_pot_png);

  if (glider == NULL) {
    fprintf (stderr, "failed to create %s glider surface\n",
	     _render_format_string (info.s->format));
    return RENDER_STATUS_FAILED;
  }

  if (info.s->npot)
    big_glider = create_surface_for_png (surface, info.s->format,
					 big_glider_npot_png);
  else
    big_glider = create_surface_for_png (surface, info.s->format,
					 big_glider_pot_png);

  if (big_glider == NULL) {
    fprintf (stderr, "failed to create %s glider surface\n",
	     _render_format_string (info.s->format));
    return RENDER_STATUS_FAILED;
  }

  solid_red = create_surface_for_data (surface,
				       RENDER_FORMAT_ARGB32,
				       (unsigned char *) &red_color,
				       1, 1);
  if (solid_red == NULL) {
    fprintf (stderr, "failed to create solid surface\n");
    return RENDER_STATUS_FAILED;
  }

  status = surface->backend->set_fill (solid_red, RENDER_FILL_REPEAT);
  if (status) {
    fprintf (stderr, "set_fill: %s\n", _render_status_string (status));
    return RENDER_STATUS_FAILED;
  }

  solid_blue = create_surface_for_data (surface,
					RENDER_FORMAT_ARGB32,
					(unsigned char *) &blue_color,
					1, 1);
  if (solid_blue == NULL) {
    fprintf (stderr, "failed to create solid surface\n");
    return RENDER_STATUS_FAILED;
  }

  status = surface->backend->set_fill (solid_blue, RENDER_FILL_REPEAT);
  if (status) {
    fprintf (stderr, "set_fill: %s\n", _render_status_string (status));
    return RENDER_STATUS_FAILED;
  }

  grad2x1 = create_surface_for_data (surface,
				     RENDER_FORMAT_ARGB32,
				     (unsigned char *) grad2x1_colors,
				     2, 1);
  if (grad2x1 == NULL) {
    fprintf (stderr, "failed to create gradient surface\n");
    return RENDER_STATUS_FAILED;
  }

  grad11x1 = create_surface_for_data (surface,
				      RENDER_FORMAT_ARGB32,
				      (unsigned char *) grad11x1_colors,
				      11, 1);
  if (grad11x1 == NULL) {
    fprintf (stderr, "failed to create gradient surface\n");
    return RENDER_STATUS_FAILED;
  }

  data = malloc (w * h * 4);
  if (!data) {
    fprintf (stderr, "out of memory\n");
    return RENDER_STATUS_FAILED;
  }

  for (y = 0; y < h; y++) {
    unsigned int *line, pixel;
    unsigned char red, green, blue;

    line = (unsigned int *) &data[y * w * 4];

    for (x = 0; x < w; x++) {
      red = green = blue = (unsigned char) ((0xff * (h - y / 2)) / h);
      pixel = 0xff000000 | (red << 16) | (green << 8) | (blue << 0);
      line[x] = pixel;
    }
  }

  info.bg = create_surface_for_data (surface,
				     RENDER_FORMAT_RGB24,
				     data, w, h);
  if (info.bg == NULL) {
    fprintf (stderr, "failed to create background surface\n");
    return RENDER_STATUS_FAILED;
  }

  free (data);

  status = _render_set_background (surface, info.bg, info.logo);
  if (status) {
    fprintf (stderr, "failed to set background: [%s]\n",
	     _render_status_string (status));
    return RENDER_STATUS_FAILED;
  }
  surface->backend->show (surface);


  /* start testing */

  if (info.s->sleep > 0)
    printf ("[sleeping %d second%s between tests]",
	    info.s->sleep, (info.s->sleep == 1)? "": "s");

  if (info.s->interactive) {
    if (!info.s->quiet)
      printf ("[press return to advance to next test]");

    getchar ();
  }

  if (TEST_CHECK) {
    status = RENDER_STATUS_SUCCESS;
    _print_info ("composite solid source\n", test, info.s);
    _render_composite_tests (&info, solid_blue, NULL, status);
  }

  if (TEST_CHECK) {
    status = RENDER_STATUS_SUCCESS;
    _print_info ("composite ARGB source\n", test, info.s);
    _render_composite_tests (&info, glider, NULL, status);
  }

  if (TEST_CHECK) {
    render_matrix_t m;

    m.m[0][0] = cos (1.8);
    m.m[0][1] = sin (0.8);
    m.m[0][2] = -20.0;

    m.m[1][0] = sin (0.8);
    m.m[1][1] = -cos (1.8);
    m.m[1][2] = -80.0;

    m.m[2][0] = 0.0;
    m.m[2][1] = 0.0;
    m.m[2][2] = 1.0;

    _print_info ("composite ARGB source with affine transform\n",
		 test, info.s);
    status = _render_set_transform (glider, &m);
    if (!status)
      status = _render_set_filter (glider, RENDER_FILTER_BILINEAR, NULL, 0);
    _render_composite_tests (&info, glider, NULL, status);
    _render_set_transform (glider, &_identity);
    _render_set_filter (glider, RENDER_FILTER_NEAREST, NULL, 0);
  }

  if (TEST_CHECK) {
    render_matrix_t m;

    m.m[0][0] = 1.0;
    m.m[0][1] = 0.0;
    m.m[0][2] = -((w * 8) / 20) + big_glider->width / 2;

    m.m[1][0] = 0.0;
    m.m[1][1] = 1.0;
    m.m[1][2] = -((h * 8) / 20) + big_glider->height / 2;

    m.m[2][0] = 0.0;
    m.m[2][1] = 0.0;
    m.m[2][2] = 1.0;

    _print_info ("composite ARGB source with bilinear filtered pixel aligned "
		 "translation\n", test, info.s);
    status = _render_set_transform (big_glider, &m);
    if (!status)
      status = _render_set_filter (big_glider, RENDER_FILTER_BILINEAR,
				   NULL, 0);
    _render_composite_tests (&info, big_glider, NULL, status);
    _render_set_transform (big_glider, &_identity);
    _render_set_filter (big_glider, RENDER_FILTER_NEAREST, NULL, 0);
  }

  if (TEST_CHECK) {
    render_matrix_t m = {
      {
	{ 1.0, 0.0, -32.0 },
	{ 0.0, -2.0, 200.0 },
	{ 0.0, 1.0 / 128.0, 1.0 },
      }
    };

    _print_info ("composite ARGB source with bilinear filtered projective "
		 "transform\n",
		 test, info.s);
    status = _render_set_transform (big_glider, &m);
    if (!status)
      status = _render_set_filter (big_glider, RENDER_FILTER_BILINEAR,
				   NULL, 0);
    _render_composite_tests (&info, big_glider, NULL, status);
    _render_set_transform (big_glider, &_identity);
    _render_set_filter (big_glider, RENDER_FILTER_NEAREST, NULL, 0);
  }

  if (TEST_CHECK) {
    status = RENDER_STATUS_SUCCESS;
    _print_info ("composite solid source in solid mask\n", test, info.s);
    _render_composite_tests (&info, solid_blue, solid_red, status);
  }

  if (TEST_CHECK) {
    _print_info ("composite solid source in component alpha solid mask\n",
		 test, info.s);
    status = surface->backend->set_component_alpha (solid_red, 1);
    _render_composite_tests (&info, solid_blue, solid_red, status);
    surface->backend->set_component_alpha (solid_red, 0);
  }

  if (TEST_CHECK) {
    status = RENDER_STATUS_SUCCESS;
    _print_info ("composite solid source in A mask\n", test, info.s);
    _render_composite_tests (&info, solid_blue, a_mask, status);
  }

  if (TEST_CHECK) {
    status = RENDER_STATUS_SUCCESS;
    _print_info ("composite solid source in ARGB mask\n", test, info.s);
    _render_composite_tests (&info, solid_blue, argb_mask, status);
  }

  if (TEST_CHECK) {
    _print_info ("composite solid source in component alpha ARGB mask\n",
		 test, info.s);
    status = surface->backend->set_component_alpha (argb_mask, 1);
    _render_composite_tests (&info, solid_blue, argb_mask, status);
    surface->backend->set_component_alpha (argb_mask, 0);
  }

  if (TEST_CHECK) {
    status = RENDER_STATUS_SUCCESS;
    _print_info ("composite ARGB source in solid mask\n", test, info.s);
    _render_composite_tests (&info, glider, solid_red, status);
  }

  if (TEST_CHECK) {
    _print_info ("composite ARGB source in component alpha solid mask\n",
		 test, info.s);
    status = surface->backend->set_component_alpha (solid_red, 1);
    _render_composite_tests (&info, glider, solid_red, status);
    surface->backend->set_component_alpha (solid_red, 0);
  }

  if (TEST_CHECK) {
    status = RENDER_STATUS_SUCCESS;
    _print_info ("composite ARGB source in A mask\n", test, info.s);
    _render_composite_tests (&info, glider, a_mask, status);
  }

  if (TEST_CHECK) {
    status = RENDER_STATUS_SUCCESS;
    _print_info ("composite ARGB source in ARGB mask\n", test, info.s);
    _render_composite_tests (&info, glider, argb_mask, status);
  }

  if (TEST_CHECK) {
    _print_info ("composite ARGB source in component alpha ARGB mask\n",
		 test, info.s);
    status = surface->backend->set_component_alpha (argb_mask, 1);
    _render_composite_tests (&info, glider, argb_mask, status);
    surface->backend->set_component_alpha (argb_mask, 0);
  }

  if (TEST_CHECK) {
    render_matrix_t m;

    m.m[0][0] = cos (0.5) * 2.0;
    m.m[0][1] = sin (0.8);
    m.m[0][2] = -10.0;

    m.m[1][0] = sin (0.8);
    m.m[1][1] = -cos (0.5) * 2.0;
    m.m[1][2] = 80.0;

    m.m[2][0] = 0.0;
    m.m[2][1] = 0.0;
    m.m[2][2] = 1.0;

    _print_info ("composite ARGB source in A mask with bilinear filtered "
		 "affine transform\n",
		 test, info.s);
    status = _render_set_transform (a_mask, &m);
    if (!status)
      status = _render_set_filter (a_mask, RENDER_FILTER_BILINEAR, NULL, 0);
    _render_composite_tests (&info, glider, a_mask, status);
    _render_set_transform (a_mask, &_identity);
    _render_set_filter (a_mask, RENDER_FILTER_NEAREST, NULL, 0);
  }

  if (TEST_CHECK) {
    render_matrix_t m;

    m.m[0][0] = cos (0.5) * 2.0;
    m.m[0][1] = sin (0.8);
    m.m[0][2] = -10.0;

    m.m[1][0] = sin (0.8);
    m.m[1][1] = -cos (0.5) * 2.0;
    m.m[1][2] = 80.0;

    m.m[2][0] = 0.0;
    m.m[2][1] = 0.0;
    m.m[2][2] = 1.0;

    _print_info ("composite ARGB source with nearest filtered affine "
		 "transform\n", test, info.s);
    status = _render_set_transform (big_glider, &m);
    _render_composite_tests (&info, big_glider, NULL, status);
    _render_set_transform (big_glider, &_identity);
  }

  if (TEST_CHECK) {
    render_matrix_t m;

    m.m[0][0] = cos (0.5) * 2.0;
    m.m[0][1] = sin (0.8);
    m.m[0][2] = -10.0;

    m.m[1][0] = sin (0.8);
    m.m[1][1] = -cos (0.5) * 2.0;
    m.m[1][2] = 80.0;

    m.m[2][0] = 0.0;
    m.m[2][1] = 0.0;
    m.m[2][2] = 1.0;

    _print_info ("composite ARGB source with bilinear filtered affine "
		 "transform\n", test, info.s);
    status = _render_set_transform (big_glider, &m);
    if (!status)
      status = _render_set_filter (big_glider, RENDER_FILTER_BILINEAR,
				   NULL, 0);
    _render_composite_tests (&info, big_glider, NULL, status);
    _render_set_transform (big_glider, &_identity);
    _render_set_filter (big_glider, RENDER_FILTER_NEAREST, NULL, 0);
  }

  if (TEST_CHECK) {
    render_fixed16_16_t radius;
    render_matrix_t m;

    m.m[0][0] = cos (0.5) * 2.0;
    m.m[0][1] = sin (0.8);
    m.m[0][2] = -10.0;

    m.m[1][0] = sin (0.8);
    m.m[1][1] = -cos (0.5) * 2.0;
    m.m[1][2] = 80.0;

    m.m[2][0] = 0.0;
    m.m[2][1] = 0.0;
    m.m[2][2] = 1.0;

    radius = DOUBLE_TO_FIXED (0.5);

    _print_info ("composite ARGB source with gaussian filtered affine "
		 "transform\n", test, info.s);
    status = _render_set_transform (big_glider, &m);
    if (!status)
      status = _render_set_filter (big_glider, RENDER_FILTER_GAUSSIAN,
				   &radius, 1);
    _render_composite_tests (&info, big_glider, NULL, status);
    _render_set_transform (big_glider, &_identity);
    _render_set_filter (big_glider, RENDER_FILTER_NEAREST, NULL, 0);
  }

  if (TEST_CHECK) {
    render_fixed16_16_t radius;
    render_matrix_t m;

    m.m[0][0] = 1.0;
    m.m[0][1] = 0.0;
    m.m[0][2] = -((w * 8) / 20) + big_glider->width / 2;

    m.m[1][0] = 0.0;
    m.m[1][1] = 1.0;
    m.m[1][2] = -((h * 8) / 20) + big_glider->height / 2;

    m.m[2][0] = 0.0;
    m.m[2][1] = 0.0;
    m.m[2][2] = 1.0;

    radius = DOUBLE_TO_FIXED (3.0);

    _print_info ("composite ARGB source with gaussian blur filter\n",
		 test, info.s);
    status = _render_set_transform (big_glider, &m);
    if (!status)
      status = _render_set_filter (big_glider, RENDER_FILTER_GAUSSIAN,
				   &radius, 1);
    _render_composite_tests (&info, big_glider, NULL, status);
    _render_set_filter (big_glider, RENDER_FILTER_NEAREST, NULL, 0);
    _render_set_transform (big_glider, &_identity);
  }

  if (TEST_CHECK) {
    render_fixed16_16_t params[11];
    render_matrix_t m;

    m.m[0][0] = 1.0;
    m.m[0][1] = 0.0;
    m.m[0][2] = -((w * 8) / 20) + big_glider->width / 2;

    m.m[1][0] = 0.0;
    m.m[1][1] = 1.0;
    m.m[1][2] = -((h * 8) / 20) + big_glider->height / 2;

    m.m[2][0] = 0.0;
    m.m[2][1] = 0.0;
    m.m[2][2] = 1.0;

    /* 3x3 kernel */
    params[0] = DOUBLE_TO_FIXED (3.0);
    params[1] = DOUBLE_TO_FIXED (3.0);

    params[2] = DOUBLE_TO_FIXED (0.0);
    params[3] = DOUBLE_TO_FIXED (-1.0);
    params[4] = DOUBLE_TO_FIXED (0.0);

    params[5] = DOUBLE_TO_FIXED (-1.0);
    params[6] = DOUBLE_TO_FIXED (5.0);
    params[7] = DOUBLE_TO_FIXED (-1.0);

    params[8] = DOUBLE_TO_FIXED (0.0);
    params[9] = DOUBLE_TO_FIXED (-1.0);
    params[10] = DOUBLE_TO_FIXED (0.0);

    _print_info ("composite ARGB source with high pass convolution filter\n",
		 test, info.s);
    status = _render_set_transform (big_glider, &m);
    if (!status)
      status = _render_set_filter (big_glider,
				   RENDER_FILTER_CONVOLUTION,
				   params,
				   sizeof (params) /
				   sizeof (render_fixed16_16_t));
    _render_composite_tests (&info, big_glider, NULL, status);
    _render_set_filter (big_glider, RENDER_FILTER_NEAREST, NULL, 0);
    _render_set_transform (big_glider, &_identity);
  }

  if (TEST_CHECK) {
    render_fixed16_16_t params[4];

    /* from */
    params[0] = DOUBLE_TO_FIXED (((w * 8) / 20) - 32.0);
    params[1] = DOUBLE_TO_FIXED (((h * 8) / 20) - 32.0);

    /* to */
    params[2] = DOUBLE_TO_FIXED (((w * 8) / 20) + 32.0);
    params[3] = DOUBLE_TO_FIXED (((h * 8) / 20) + 32.0);

    /* using default color stop parameters */

    _print_info ("composite ARGB source with linear gradient filter "
		 "(2 color stops)\n",
		 test, info.s);
    status = _render_set_filter (grad2x1,
				 RENDER_FILTER_LINEAR_GRADIENT,
				 params,
				 sizeof (params) /
				 sizeof (render_fixed16_16_t));
    _render_composite_tests (&info, grad2x1, NULL, status);
    _render_set_filter (grad2x1, RENDER_FILTER_NEAREST, NULL, 0);
  }

  if (TEST_CHECK) {
    render_fixed16_16_t params[] = {
      DOUBLE_TO_FIXED (0.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.0), DOUBLE_TO_FIXED (0.0),

      DOUBLE_TO_FIXED (0.00000), DOUBLE_TO_FIXED (0.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.34892), DOUBLE_TO_FIXED (1.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.53255), DOUBLE_TO_FIXED (2.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.54257), DOUBLE_TO_FIXED (3.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.55593), DOUBLE_TO_FIXED (4.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.58264), DOUBLE_TO_FIXED (5.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.61269), DOUBLE_TO_FIXED (6.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.70284), DOUBLE_TO_FIXED (7.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.94825), DOUBLE_TO_FIXED (8.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.97412), DOUBLE_TO_FIXED (9.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (1.00000), DOUBLE_TO_FIXED (10.0), DOUBLE_TO_FIXED (0.0)
    };

    /* from */
    params[0] = DOUBLE_TO_FIXED (0.0);
    params[1] = DOUBLE_TO_FIXED ((h * 8) / 20 - 64.0);

    /* to */
    params[2] = DOUBLE_TO_FIXED (0.0);
    params[3] = DOUBLE_TO_FIXED ((h * 8) / 20 + 64.0);

    _print_info ("composite ARGB source with linear gradient filter "
		 "(11 color stops)\n",
		 test, info.s);
    status = _render_set_filter (grad11x1,
				 RENDER_FILTER_LINEAR_GRADIENT,
				 params,
				 sizeof (params) /
				 sizeof (render_fixed16_16_t));
    _render_composite_tests (&info, grad11x1, NULL, status);
    _render_set_filter (grad11x1, RENDER_FILTER_NEAREST, NULL, 0);
  }

  if (TEST_CHECK) {
    render_matrix_t m;

    m.m[0][0] = 1.0 / ((w * 8) / 10);
    m.m[0][1] = 0.0;
    m.m[0][2] = 0.0;

    m.m[1][0] = 0.0;
    m.m[1][1] = 1.0 / ((h * 8) / 20);
    m.m[1][2] = -0.5;

    m.m[2][0] = 0.0;
    m.m[2][1] = 0.0;
    m.m[2][2] = 1.0;

    /* using default normalized gradient parameters and default
       color stop parameters */

    _print_info ("composite ARGB source with radial gradient filter "
		 "(2 color stops)\n",
		 test, info.s);
    status = _render_set_transform (grad2x1, &m);
    if (!status)
      status = _render_set_filter (grad2x1,
				   RENDER_FILTER_RADIAL_GRADIENT,
				   NULL, 0);
    _render_composite_tests (&info, grad2x1, NULL, status);
    _render_set_filter (grad2x1, RENDER_FILTER_NEAREST, NULL, 0);
    _render_set_transform (grad2x1, &_identity);
  }

    if (TEST_CHECK) {
    render_matrix_t m = {
      {
	{ 1.0, 0.0, -32.0 },
	{ 0.0, -2.0, 200.0 },
	{ 0.0, 1.0 / 48.0, 1.0 },
      }
    };
    render_fixed16_16_t params[6];

    /* center0 */
    params[0] = DOUBLE_TO_FIXED (32.0);
    params[1] = DOUBLE_TO_FIXED (32.0);

    /* radius0 */
    params[2] = DOUBLE_TO_FIXED (0.0);

    /* center1 */
    params[3] = DOUBLE_TO_FIXED (32.0);
    params[4] = DOUBLE_TO_FIXED (32.0);

    /* radius1 */
    params[5] = DOUBLE_TO_FIXED (32.0);

    _print_info ("composite ARGB source with radial gradient filter "
		 "(2 color stops) and \n    projective transform\n",
		 test, info.s);
    status = _render_set_transform (grad2x1, &m);
    if (!status)
      status = _render_set_filter (grad2x1,
				   RENDER_FILTER_RADIAL_GRADIENT,
				   params,
				   sizeof (params) /
				   sizeof (render_fixed16_16_t));
    _render_composite_tests (&info, grad2x1, NULL, status);
    _render_set_filter (grad2x1, RENDER_FILTER_NEAREST, NULL, 0);
    _render_set_transform (grad2x1, &_identity);
  }

  if (TEST_CHECK) {
    render_matrix_t m;
    render_fixed16_16_t params[] = {
      DOUBLE_TO_FIXED (0.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.0), DOUBLE_TO_FIXED (0.0),

      DOUBLE_TO_FIXED (0.00000), DOUBLE_TO_FIXED (0.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.34892), DOUBLE_TO_FIXED (1.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.53255), DOUBLE_TO_FIXED (2.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.54257), DOUBLE_TO_FIXED (3.0) ,DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.55593), DOUBLE_TO_FIXED (4.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.58264), DOUBLE_TO_FIXED (5.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.61269), DOUBLE_TO_FIXED (6.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.70284), DOUBLE_TO_FIXED (7.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.94825), DOUBLE_TO_FIXED (8.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (0.97412), DOUBLE_TO_FIXED (9.0), DOUBLE_TO_FIXED (0.0),
      DOUBLE_TO_FIXED (1.00000), DOUBLE_TO_FIXED (10.0), DOUBLE_TO_FIXED (0.0)
    };

    /* center0 */
    params[0] = DOUBLE_TO_FIXED (0.35);
    params[1] = DOUBLE_TO_FIXED (0.35);

    /* radius0 */
    params[2] = DOUBLE_TO_FIXED (0.0);

    /* center1 */
    params[3] = DOUBLE_TO_FIXED (0.5);
    params[4] = DOUBLE_TO_FIXED (0.5);

    /* radius1 */
    params[5] = DOUBLE_TO_FIXED (0.5);

    m.m[0][0] = 1.0 / ((w * 8) / 10);
    m.m[0][1] = 0.0;
    m.m[0][2] = 0.0;

    m.m[1][0] = 0.0;
    m.m[1][1] = 1.0 / ((h * 8) / 10);
    m.m[1][2] = 0.0;

    m.m[2][0] = 0.0;
    m.m[2][1] = 0.0;
    m.m[2][2] = 1.0;

    _print_info ("composite ARGB source with radial gradient filter "
		 "(11 color stops)\n",
		 test, info.s);
    status = _render_set_transform (grad11x1, &m);
    if (!status)
      status = _render_set_filter (grad11x1,
				   RENDER_FILTER_RADIAL_GRADIENT,
				   params,
				   sizeof (params) /
				   sizeof (render_fixed16_16_t));
    _render_composite_tests (&info, grad11x1, NULL, status);
    _render_set_filter (grad11x1, RENDER_FILTER_NEAREST, NULL, 0);
    _render_set_transform (grad11x1, &_identity);
  }

  if (TEST_CHECK) {
    render_fixed16_16_t params[11];
    int i;

    /* 3x3 kernel */
    params[0] = DOUBLE_TO_FIXED (3.0);
    params[1] = DOUBLE_TO_FIXED (3.0);

    for (i = 2; i < 11; i++)
      params[i] = DOUBLE_TO_FIXED (1.0);

    _print_info ("composite solid source in convolution filtered A mask\n",
		 test, info.s);
    status = _render_set_filter (a_mask,
				 RENDER_FILTER_CONVOLUTION,
				 params,
				 sizeof (params) /
				 sizeof (render_fixed16_16_t));
    _render_composite_tests (&info, solid_blue, a_mask, status);
    _render_set_filter (a_mask, RENDER_FILTER_NEAREST, NULL, 0);
  }

  if (TEST_CHECK) {
    render_fixed16_16_t params[51];
    int i;

    /* 7x7 kernel */
    params[0] = DOUBLE_TO_FIXED (7.0);
    params[1] = DOUBLE_TO_FIXED (7.0);

    for (i = 2; i < 51; i++)
      params[i] = DOUBLE_TO_FIXED (1.0);

    _print_info ("composite ARGB source in convolution filtered ARGB mask\n",
		 test, info.s);
    status = _render_set_filter (argb_mask,
				 RENDER_FILTER_CONVOLUTION,
				 params,
				 sizeof (params) /
				 sizeof (render_fixed16_16_t));
    _render_composite_tests (&info, glider, argb_mask, status);
    _render_set_filter (argb_mask, RENDER_FILTER_NEAREST, NULL, 0);
  }

  surface->backend->destroy (grad2x1);
  surface->backend->destroy (grad11x1);
  surface->backend->destroy (solid_blue);
  surface->backend->destroy (solid_red);
  surface->backend->destroy (argb_mask);
  surface->backend->destroy (big_glider);
  surface->backend->destroy (glider);
  surface->backend->destroy (a_mask);

  surface->backend->destroy (info.bg);
  surface->backend->destroy (info.logo);

  if (!info.s->quiet)
    printf ("\n");

  printf ("  success:       %3d\n", info.success);
  printf ("  not supported: %3d\n", info.not_supported);
  printf ("  failed:        %3d\n", info.failed);

  return RENDER_STATUS_SUCCESS;
}
