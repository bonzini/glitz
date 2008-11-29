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

#ifndef RENDERTEST_H_INCLUDED
#define RENDERTEST_H_INCLUDED

#define RENDER_DEFAULT_DST_WIDTH 320
#define RENDER_DEFAULT_DST_HEIGHT 280

typedef int render_bool_t;
typedef int render_fixed16_16_t;

#define FIXED_BITS 16

#define FIXED_TO_INT(f) (int) ((f) >> FIXED_BITS)
#define INT_TO_FIXED(i) ((render_fixed16_16_t) (i) << FIXED_BITS)
#define FIXED_E ((render_fixed16_16_t) 1)
#define FIXED1 (INT_TO_FIXED (1))
#define FIXED1_MINUS_E (FIXED1 - FIXED_E)
#define FIXED_FRAC(f) ((f) & FIXED1_MINUS_E)
#define FIXED_FLOOR(f) ((f) & ~FIXED1_MINUS_E)
#define FIXED_CEIL(f) FIXED_FLOOR ((f) + FIXED1_MINUS_E)

#define FIXED_FRACTION(f) ((f) & FIXED1_MINUS_E)
#define FIXED_MOD2(f) ((f) & (FIXED1 | FIXED1_MINUS_E))

#define FIXED_TO_DOUBLE(f) (((double) (f)) / 65536)
#define DOUBLE_TO_FIXED(f) ((int) ((f) * 65536))

typedef enum {
  RENDER_STATUS_SUCCESS,
  RENDER_STATUS_NOT_SUPPORTED,
  RENDER_STATUS_FAILED,
  RENDER_STATUS_NO_MEMORY
} render_status_t;

typedef enum {
  RENDER_FORMAT_A1,
  RENDER_FORMAT_A8,
  RENDER_FORMAT_RGB24,
  RENDER_FORMAT_ARGB32,
  RENDER_FORMAT_YV12,
  RENDER_FORMAT_YUY2
} render_format_t;

typedef enum {
  RENDER_OPERATOR_CLEAR,
  RENDER_OPERATOR_SRC,
  RENDER_OPERATOR_DST,
  RENDER_OPERATOR_OVER,
  RENDER_OPERATOR_OVER_REVERSE,
  RENDER_OPERATOR_IN,
  RENDER_OPERATOR_IN_REVERSE,
  RENDER_OPERATOR_OUT,
  RENDER_OPERATOR_OUT_REVERSE,
  RENDER_OPERATOR_ATOP,
  RENDER_OPERATOR_ATOP_REVERSE,
  RENDER_OPERATOR_XOR,
  RENDER_OPERATOR_ADD
} render_operator_t;

typedef enum {
  RENDER_CLIP_NONE,
  RENDER_CLIP_RECTANGLES,
  RENDER_CLIP_TRAPEZOIDS
} render_clip_t;

typedef enum {
  RENDER_FILL_NONE,
  RENDER_FILL_TRANSPARENT,
  RENDER_FILL_NEAREST,
  RENDER_FILL_REPEAT,
  RENDER_FILL_REFLECT
} render_fill_t;

typedef struct render_matrix {
  double m[3][3];
} render_matrix_t;

typedef enum {
  RENDER_FILTER_NEAREST,
  RENDER_FILTER_BILINEAR,
  RENDER_FILTER_CONVOLUTION,
  RENDER_FILTER_GAUSSIAN,
  RENDER_FILTER_LINEAR_GRADIENT,
  RENDER_FILTER_RADIAL_GRADIENT
} render_filter_t;

typedef struct render_rectangle {
  short x, y;
  unsigned short width, height;
} render_rectangle_t;

typedef struct render_span_fixed {
  render_fixed16_16_t left, right, y;
} render_span_fixed_t;

typedef struct render_trapezoid {
  render_span_fixed_t top, bottom;
} render_trapezoid_t;

typedef struct render_color {
  unsigned short red;
  unsigned short green;
  unsigned short blue;
  unsigned short alpha;
} render_color_t;

typedef struct render_backend render_backend_t;

typedef struct render_surface {
  const render_backend_t *backend;

  void *surface;
  int width;
  int height;
  unsigned long flags;
} render_surface_t;

#define RENDER_SURFACE_FLAG_SOLID_MASK     (1L << 0)
#define RENDER_SURFACE_FLAG_TRANSFORM_MASK (1L << 1)
#define RENDER_SURFACE_FLAG_FILTER_MASK    (1L << 2)

#define SURFACE_SOLID(surface) \
  ((surface)->flags & RENDER_SURFACE_FLAG_SOLID_MASK)

#define SURFACE_TRANSFORM(surface) \
  ((surface)->flags & RENDER_SURFACE_FLAG_TRANSFORM_MASK)

#define SURFACE_FILTER(surface) \
  ((surface)->flags & RENDER_SURFACE_FLAG_FILTER_MASK)

struct render_backend {
  render_surface_t *(*create_similar)      (render_surface_t *other,
					    render_format_t format,
					    int width,
					    int height);

  void              (*destroy)             (render_surface_t *surface);

  render_status_t   (*composite)           (render_operator_t op,
					    render_surface_t *src,
					    render_surface_t *mask,
					    render_surface_t *dst,
					    int x_src,
					    int y_src,
					    int x_mask,
					    int y_mask,
					    int x_dst,
					    int y_dst,
					    int width,
					    int height);

  render_status_t   (*set_pixels)          (render_surface_t *dst,
					    render_format_t format,
					    unsigned char *data);

  void              (*show)                (render_surface_t *dst);

  render_status_t   (*set_fill)            (render_surface_t *surface,
					    render_fill_t fill);

  render_status_t   (*set_component_alpha) (render_surface_t *surface,
					    render_bool_t component_alpha);

  render_status_t   (*set_transform)       (render_surface_t *surface,
					    render_matrix_t *matrix);

  render_status_t   (*set_filter)          (render_surface_t *surface,
					    render_filter_t filter,
					    render_fixed16_16_t *params,
					    int n_params);

  render_status_t   (*set_clip_rectangles) (render_surface_t *surface,
					    int x_offset,
					    int y_offset,
					    render_rectangle_t *rects,
					    int n_rects);

  render_status_t   (*set_clip_trapezoids) (render_surface_t *surface,
					    int x_offset,
					    int y_offset,
					    render_trapezoid_t *traps,
					    int n_traps);
};


/* rendertest.c */

typedef struct render_settings_t {
  int sleep;
  int interactive;
  int npot;
  int quiet;
  int first_test, last_test;
  int repeat;
  int time;
  render_operator_t op;
  render_clip_t clip;
  render_format_t format;
} render_settings_t;

int
render_run (render_surface_t *surface, render_settings_t *settings);


/* png.c */

int
render_read_png (unsigned char *buffer,
		 unsigned int *width,
		 unsigned int *height,
		 unsigned char **data);


/* args.c */

typedef struct render_arg_state {
  render_settings_t settings;
  void *pointer;
} render_arg_state_t;

typedef struct render_option {
  const char *name;
  int key;
  const char *arg;
  unsigned long flags;
  const char *doc;
} render_option_t;

typedef int (*render_parser_t) (int key, char *arg, render_arg_state_t *state);

int
render_parse_arguments (render_parser_t parser,
			const render_option_t *backend_options,
			render_arg_state_t *state,
			int argc, char **argv);

#endif /* RENDERTEST_H_INCLUDED */
