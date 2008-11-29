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

#ifndef GLITZ_H_INCLUDED
#define GLITZ_H_INCLUDED

#define GLITZ_MAJOR    0
#define GLITZ_MINOR    5
#define GLITZ_REVISION 7

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef int glitz_bool_t;
typedef short glitz_short_t;
typedef int glitz_int_t;
typedef float glitz_float_t;
typedef double glitz_double_t;
typedef int glitz_fixed16_16_t;

/**
 * glitz_drawable_t:
 *
 * A drawable provides the infrastructure for targets of OpenGL drawing;
 * drawables can be native (backend-specific) drawing targets such as
 * pbuffers, windows, or pixmaps, or they can be framebuffer objects,
 * which are additional offscreen drawables added to a native drawables.
 * Unlike surfaces, drawables can be created with a depth buffer, stencil
 * buffer and multisampling support.  Drawables can also be
 * double-buffered.
 *
 * To use drawables for 3D drawing, you have to create a glitz_context,
 * make it current, and then perform normal OpenGL drawing on it.
 *
 * To use drawables for Glitz compositing, either as sources or
 * destinations, you have to create a glitz_surface and attach it to
 * the drawable using glitz_surface_attach.  For double-buffered
 * drawables, it is possible to attach a different surface to each
 * buffer.
 **/
typedef struct _glitz_drawable glitz_drawable_t;

/**
 * A surface is a texture, augmented with the state needed to use it
 * as either a drawing target (which is done by attaching a drawable
 * to it) or as a compositing source.
 *
 * Once attached to a drawable, the surface can be used as a target
 * for either 3D drawing or Glitz compositing.  In the former case,
 * you have to also create a glitz_context for the attached drawable.
 * In the latter case, you can pass the surface directly to Glitz's
 * compositing function.
 **/
typedef struct _glitz_surface glitz_surface_t;

/**
 * glitz_rectangle_t:
 * @x: X coordinate of the left side of the rectangle
 * @y: Y coordinate of the top side of the rectangle
 * @width: width of the rectangle
 * @height: height of the rectangle
 *
 * A data structure for holding a rectangle as an origin/extent pair.
 **/
typedef struct _glitz_rectangle_t {
  short          x, y;
  unsigned short width, height;
} glitz_rectangle_t;

/**
 * glitz_rectangle_t:
 * @x1: X coordinate of the left side of the rectangle
 * @y1: Y coordinate of the top side of the rectangle
 * @x2: X coordinate of the right side of the rectangle
 * @y2: Y coordinate of the bottom side of the rectangle
 *
 * A data structure for holding a rectangle as a pair of corners.
 **/
typedef struct _glitz_box_t {
  short x1, y1, x2, y2;
} glitz_box_t;

/**
 * glitz_point_fixed_t:
 * @x: X coordinate of the point, in 16.16 fixed point
 * @y: Y coordinate of the point, in 16.16 fixed point
 *
 * A data structure for holding a point.
 **/
typedef struct _glitz_point_fixed_t {
  glitz_fixed16_16_t x;
  glitz_fixed16_16_t y;
} glitz_point_fixed_t;

/**
 * glitz_line_fixed_t:
 * @p1: first endpoint, in 16.16 fixed point
 * @p2: second endpoint
 *
 * A data structure for holding a line.
 **/
typedef struct _glitz_line_fixed_t {
  glitz_point_fixed_t p1;
  glitz_point_fixed_t p2;
} glitz_line_fixed_t;

/**
 * glitz_trapezoid_t:
 * @top: Y coordinate of the top side of the trapezoid
 * @bottom: Y coordinate of the bottom side of the trapezoid
 * @left: Line describing the left side of the trapezoid
 * @right: Line describing the right side of the trapezoid
 *
 * A data structure for holding a trapezoid, described through
 * the two vertical endpoints and two lines that include the
 * non-horizontal sides.
 **/
typedef struct _glitz_trapezoid_t {
  glitz_fixed16_16_t top, bottom;
  glitz_line_fixed_t left, right;
} glitz_trapezoid_t;

/**
 * glitz_span_t:
 * @left: X coordinate of the left endpoint
 * @right: X coordinate of the right endpoint
 * @y: Y coordinate of the line
 *
 * A data structure for holding a span, i.e. an horizontal line.
 **/
typedef struct _glitz_span_fixed_t {
  glitz_fixed16_16_t left, right, y;
} glitz_span_fixed_t;

/**
 * glitz_trap_t:
 * @top: Span defining the top vertices of the trapezoid
 * @bottom: Span defining the bottom vertices of the trapezoid
 *
 * A data structure for holding a trapezoid, described through
 * the positions of the four vertices (with the equal Y values
 * specified only once).
 **/
typedef struct _glitz_trap_t {
  glitz_span_fixed_t top, bottom;
} glitz_trap_t;

/**
 * glitz_transform_t:
 * @matrix: The 3x3 matrix defining the transform.
 *
 * A #glitz_transform_t holds a projective transformation
 * on a point (x, y, u) which corresponds to Cartesian
 * coordinates (x/u, y/u).
 **/
typedef struct _glitz_transform_t {
  glitz_fixed16_16_t matrix[3][3];
} glitz_transform_t;

typedef struct {
  unsigned short red;
  unsigned short green;
  unsigned short blue;
  unsigned short alpha;
} glitz_color_t;

/**
 * glitz_filter_t:
 * @GLITZ_FILTER_NEAREST: specifies that the source is interpolated
 * to the nearest-pixel.
 * @GLITZ_FILTER_BILINEAR: specifies that the source is interpolated
 * using bilinear filter.
 * @GLITZ_FILTER_CONVOLUTION: specifies that the source is subject
 * to a convolution operation.  The first two parameters specify the
 * convolution kernel's size (note: they are fixed point too!), the remaining
 * parameters give the matrix in column-major order starting at the
 * top-left corner.
 * @GLITZ_FILTER_GAUSSIAN: specifies that the source is subject
 * to a Gaussian blur filter.  The first two parameters specify the
 * radius of the filter and the variance (sigma), defaulting to the
 * radius divided by two.  The last specifies the half-size of the
 * filter, defaulting to the radius, and is used to compute the number
 * of taps in the filter (which will be 2*halfsize+1).
 * @GLITZ_FILTER_LINEAR_GRADIENT: specifies that the source is being
 * used to provide the color stops of a linear gradient.  The first
 * four parameters specifies the endpoints of the gradient, outside
 * which the gradient coordinate will be clamped respectively to 0
 * and 1.  The other parameters are triples that map a gradient
 * coordinate to a pair of texture coordinate.  One of the two
 * texture coordinates is typically 0.
 * @GLITZ_FILTER_RADIAL_GRADIENT: specifies that the source is being
 * used to provide the color stops of a radial gradient.  The first
 * six parameters specifies the endpoints of the gradient as two
 * (xcenter, ycenter, radius) triples.  Within the first circle
 * and outside the second circle the gradient coordinate will be
 * clamped respectively to 0 and 1.
 * The other parameters are triples that map a gradient coordinate
 * to a pair of texture coordinate.  One of the two texture coordinates
 * is typically 0.
 *
 * #glitz_filter_t is used to build a fragment program providing
 * image processing effects.  The maximum number of parameters depends
 * on the capabilities of the underlying OpenGL implementation.
 **/
typedef enum {
  GLITZ_FILTER_NEAREST,
  GLITZ_FILTER_BILINEAR,
  GLITZ_FILTER_CONVOLUTION,
  GLITZ_FILTER_GAUSSIAN,
  GLITZ_FILTER_LINEAR_GRADIENT,
  GLITZ_FILTER_RADIAL_GRADIENT
} glitz_filter_t;

/**
 * glitz_operator_t:
 * @GLITZ_OPERATOR_CLEAR: clear destination layer (bounded)
 * @GLITZ_OPERATOR_SRC: replace destination layer (bounded)
 * @GLITZ_OPERATOR_OVER: draw source layer on top of destination layer
 * (bounded)
 * @GLITZ_OPERATOR_IN: draw source where there was destination content
 * (unbounded)
 * @GLITZ_OPERATOR_OUT: draw source where there was no destination
 * content (unbounded)
 * @GLITZ_OPERATOR_ATOP: draw source on top of destination content and
 * only there
 * @GLITZ_OPERATOR_DST: ignore the source
 * @GLITZ_OPERATOR_OVER_REVERSE: draw destination on top of source
 * @GLITZ_OPERATOR_IN_REVERSE: leave destination only where there was
 * source content (unbounded)
 * @GLITZ_OPERATOR_OUT_REVERSE: leave destination only where there was no
 * source content
 * @GLITZ_OPERATOR_ATOP_REVERSE: leave destination on top of source content
 * and only there (unbounded)
 * @GLITZ_OPERATOR_XOR: source and destination are shown where there is only
 * one of them
 * @GLITZ_OPERATOR_ADD: source and destination layers are accumulated
 * @GLITZ_OPERATOR_SATURATE: like over, but assuming source and dest are
 * disjoint geometries
 *
 * #glitz_operator_t is used to set the compositing operator for all Glitz
 * compositing operations.
 *
 * The operators marked as <firstterm>unbounded</firstterm> modify their
 * destination even outside of the mask layer (that is, their effect is not
 * bound by the mask layer).  However, their effect can still be limited by
 * way of clipping.
 *
 * To keep things simple, the operator descriptions here
 * document the behavior for when both source and destination are either fully
 * transparent or fully opaque.  The actual implementation works for
 * translucent layers too.
 * For a more detailed explanation of the effects of each operator, including
 * the mathematical definitions, refer to the original paper by Porter and
 * Duff.
 **/
typedef enum {
  GLITZ_OPERATOR_CLEAR,
  GLITZ_OPERATOR_SRC,
  GLITZ_OPERATOR_DST,
  GLITZ_OPERATOR_OVER,
  GLITZ_OPERATOR_OVER_REVERSE,
  GLITZ_OPERATOR_IN,
  GLITZ_OPERATOR_IN_REVERSE,
  GLITZ_OPERATOR_OUT,
  GLITZ_OPERATOR_OUT_REVERSE,
  GLITZ_OPERATOR_ATOP,
  GLITZ_OPERATOR_ATOP_REVERSE,
  GLITZ_OPERATOR_XOR,
  GLITZ_OPERATOR_ADD
} glitz_operator_t;

/**
 * glitz_pixel_scanline_order_t:
 * @GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN: The axes intersect at the
 * top-left corner of the framebuffer.
 * @GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP: The axes intersect at the
 * bottom-left corner of the framebuffer.
 *
 * #glitz_pixel_scanline_order_t specifies the orientation of the Y
 * axis in a drawable.
 **/
typedef enum {
  GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN,
  GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP
} glitz_pixel_scanline_order_t;

typedef enum {
  GLITZ_FEATURE_TEXTURE_RECTANGLE_MASK        = (1L <<  0),
  GLITZ_FEATURE_TEXTURE_NON_POWER_OF_TWO_MASK = (1L <<  1),
  GLITZ_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK  = (1L <<  2),
  GLITZ_FEATURE_TEXTURE_BORDER_CLAMP_MASK     = (1L <<  3),
  GLITZ_FEATURE_MULTISAMPLE_MASK              = (1L <<  4),
  GLITZ_FEATURE_MULTISAMPLE_FILTER_HINT_MASK  = (1L <<  5),
  GLITZ_FEATURE_MULTITEXTURE_MASK             = (1L <<  6),
  GLITZ_FEATURE_TEXTURE_ENV_COMBINE_MASK      = (1L <<  7),
  GLITZ_FEATURE_TEXTURE_ENV_DOT3_MASK         = (1L <<  8),
  GLITZ_FEATURE_FRAGMENT_PROGRAM_MASK         = (1L <<  9),
  GLITZ_FEATURE_VERTEX_BUFFER_OBJECT_MASK     = (1L << 10),
  GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK      = (1L << 11),
  GLITZ_FEATURE_PER_COMPONENT_RENDERING_MASK  = (1L << 12),
  GLITZ_FEATURE_BLEND_COLOR_MASK              = (1L << 13),
  GLITZ_FEATURE_PACKED_PIXELS_MASK            = (1L << 14),
  GLITZ_FEATURE_MULTI_DRAW_ARRAYS_MASK        = (1L << 15),
  GLITZ_FEATURE_FRAMEBUFFER_OBJECT_MASK       = (1L << 16),
  GLITZ_FEATURE_COPY_SUB_BUFFER_MASK          = (1L << 17),
  GLITZ_FEATURE_DIRECT_RENDERING_MASK         = (1L << 18),
  GLITZ_FEATURE_TEXTURE_FROM_PIXMAP_MASK      = (1L << 19)
} glitz_feature_t;

/* glitz_format.c */

/**
 * glitz_format_name_t:
 * @GLITZ_STANDARD_ARGB32: each pixel is a 32-bit quantity, with
 *   alpha in the upper 8 bits, then red, then green, then blue.
 *   The 32-bit quantities are stored native-endian. Pre-multiplied
 *   alpha is used. (That is, 50% transparent red is 0x80800000,
 *   not 0x80ff0000.)
 * @GLITZ_STANDARD_RGB24: each pixel is a 32-bit quantity, with
 *   the upper 8 bits unused. Red, Green, and Blue are stored
 *   in the remaining 24 bits in that order.
 * @GLITZ_STANDARD_A8: each pixel is a 8-bit quantity holding
 *   an alpha value.
 * @GLITZ_STANDARD_A1: each pixel is a 1-bit quantity holding
 *   an alpha value. Pixels are packed together into 32-bit
 *   quantities. The ordering of the bits matches the
 *   endianess of the platform. On a big-endian machine, the
 *   first pixel is in the uppermost bit, on a little-endian
 *   machine the first pixel is in the least-significant bit.
 *
 * #glitz_format_name_t is used to identify standard formats for
 * surface data.
 *
 * New entries may be added in future versions.
 **/
typedef enum {
  GLITZ_STANDARD_ARGB32,
  GLITZ_STANDARD_RGB24,
  GLITZ_STANDARD_A8,
  GLITZ_STANDARD_A1
} glitz_format_name_t;

/**
 * glitz_format_mask_t:
 * @GLITZ_FORMAT_ID_MASK: specifies that the program requests
 * the exact surface or drawable format whose id is given.
 * @GLITZ_FORMAT_RED_SIZE_MASK: specifies that the program requests
 * a surface or drawable format with the given number of bits per
 * pixel in the red component.
 * @GLITZ_FORMAT_GREEN_SIZE_MASK: specifies that the program requests
 * a surface or drawable format with the given number of bits per
 * pixel in the green component.
 * @GLITZ_FORMAT_BLUE_SIZE_MASK: specifies that the program requests
 * a surface or drawable format with the given number of bits per
 * pixel in the blue component.
 * @GLITZ_FORMAT_ALPHA_SIZE_MASK: specifies that the program requests
 * a surface or drawable format with the given number of bits per
 * pixel in the alpha component.
 * @GLITZ_FORMAT_FOURCC_MASK: specifies that the program requests
 * a surface or drawable format with the given color components.
 * @GLITZ_FORMAT_DEPTH_SIZE_MASK: specifies that the program requests
 * a drawable format with at least the given number of bits per pixel
 * in the depth buffer.
 * @GLITZ_FORMAT_STENCIL_SIZE_MASK: specifies that the program requests
 * a drawable format with at least the given number of bits per pixel
 * in the stencil buffer.
 * @GLITZ_FORMAT_DOUBLEBUFFER_MASK: specifies that the program requires
 * the drawable format to match its request for presence or absence of
 * a back buffer.
 * @GLITZ_FORMAT_SAMPLES_MASK: specifies that the program requests
 * a drawable format with the given supersampling characteristics.
 * @GLITZ_FORMAT_SCANLINE_ORDER_MASK: specifies that the program requests
 * a drawable format with the given Y axis direction.
 * @GLITZ_FORMAT_DEPTH_MASK: specifies that the program requires the
 * drawable format to match the requested depth (red size+green size+
 * blue size).  Only used for GLX, should not be used alone if portability
 * to other backends is desired.
 *
 * Multiple defined values for #glitz_format_mask_t can be ORed together
 * to form a mask.  The mask tells Glitz which fields of a
 * #glitz_format_t or #glitz_drawable_format_t are significant.
 **/
typedef enum {
  GLITZ_FORMAT_ID_MASK             = (1L <<  0),
  GLITZ_FORMAT_RED_SIZE_MASK       = (1L <<  1),
  GLITZ_FORMAT_GREEN_SIZE_MASK     = (1L <<  2),
  GLITZ_FORMAT_BLUE_SIZE_MASK      = (1L <<  3),
  GLITZ_FORMAT_ALPHA_SIZE_MASK     = (1L <<  4),
  GLITZ_FORMAT_FOURCC_MASK         = (1L <<  5),
  GLITZ_FORMAT_DEPTH_SIZE_MASK     = (1L <<  6),
  GLITZ_FORMAT_STENCIL_SIZE_MASK   = (1L <<  7),
  GLITZ_FORMAT_DOUBLEBUFFER_MASK   = (1L <<  8),
  GLITZ_FORMAT_SAMPLES_MASK        = (1L <<  9),
  GLITZ_FORMAT_SCANLINE_ORDER_MASK = (1L <<  10),
  GLITZ_FORMAT_DEPTH_MASK          = (1L <<  11),
  GLITZ_DRAWABLE_FORMAT_ALL_EXCEPT_ID_MASK = (1L << 12) - 2
} glitz_format_mask_t;

typedef unsigned long glitz_format_id_t;

typedef unsigned int glitz_fourcc_t;

#define GLITZ_FOURCC(a, b, c, d)                                \
    ((a) | (b) << 8 | (c) << 16 | ((glitz_fourcc_t) (d)) << 24)

#define GLITZ_FOURCC_RGB  ((glitz_fourcc_t) 0x0)
#define GLITZ_FOURCC_YV12 GLITZ_FOURCC ('Y', 'V', '1', '2')
#define GLITZ_FOURCC_YUY2 GLITZ_FOURCC ('Y', 'U', 'Y', '2')

/**
 * @glitz_color_format_t:
 * @fourcc: The desired color space.
 * @red_size: The desired number of bits per pixel in the red component.
 * @green_size: The desired number of bits per pixel in the green component.
 * @blue_size: The desired number of bits per pixel in the blue component.
 * @alpha_size: The desired number of bits per pixel in the alpha component.
 *
 * A #glitz_color_format_t is the common part of surface and
 * drawable formats.
 **/
typedef struct _glitz_color_format_t {
  glitz_fourcc_t fourcc;
  unsigned short red_size;
  unsigned short green_size;
  unsigned short blue_size;
  unsigned short alpha_size;
} glitz_color_format_t;

/**
 * glitz_drawable_format_t:
 * @id: specifies a unique name for each drawable format.
 * @color: specifies the color space and per-component depth.
 * @depth: specifies the overall depth.
 * @depth_size: specifies the number of bits per pixel in the depth buffer.
 * @stencil_size: specifies the number of bits per pixel in the stencil buffer.
 * @samples: specifies the size of the supersampling grid for antialiasing.
 * @doublebuffer: specifies whether a back buffer is desired.
 * @scanline_order: specifies that the Y axis direction.
 *
 * A #glitz_drawable_format_t specifies the desired characteristics of a
 * #glitz_drawable_t, including the presence of depth and stencil buffers,
 * and of a back buffer.
 **/
typedef struct _glitz_drawable_format_t {
  glitz_format_id_t        id;
  glitz_color_format_t     color;
  unsigned short           depth_size;
  unsigned short           stencil_size;
  unsigned short           samples;
  unsigned short           doublebuffer; /* glitz_bool_t */
  unsigned short           depth;
  unsigned short           scanline_order; /* glitz_pixel_scanline_order_t */
} glitz_drawable_format_t;

/**
 * glitz_format_t:
 * @id: specifies a unique name for each drawable format.
 * @color: specifies the color space and per-component depth.
 *
 * A #glitz_format_t specifies the desired characteristics of a
 * #glitz_surface_t.  A surface is only used as a source, and so it does
 * not need to support depth buffers, stencil buffers, antialiasing
 * and double buffering.
 **/
typedef struct _glitz_format_t {
  glitz_format_id_t    id;
  glitz_color_format_t color;
} glitz_format_t;

glitz_format_t *
glitz_find_standard_format (glitz_drawable_t    *drawable,
			    glitz_format_name_t format_name);

glitz_format_t *
glitz_find_format (glitz_drawable_t     *drawable,
		   unsigned long        mask,
		   const glitz_format_t *templ,
		   int                  count);

glitz_drawable_format_t *
glitz_find_drawable_format (glitz_drawable_t              *other,
			    unsigned long                 mask,
			    const glitz_drawable_format_t *templ,
			    int                           count);

glitz_drawable_format_t *
glitz_find_pbuffer_format (glitz_drawable_t              *other,
			   unsigned long                 mask,
			   const glitz_drawable_format_t *templ,
			   int                           count);

glitz_drawable_format_t *
glitz_find_pixmap_format (glitz_drawable_t              *other,
			  unsigned long                 mask,
			  const glitz_drawable_format_t *templ,
			  int                           count);

/* glitz_status.c */

/**
 * glitz_status_t:
 * @GLITZ_STATUS_SUCCESS: No error.
 * @GLITZ_STATUS_NO_MEMORY: Out of memory.
 * @GLITZ_STATUS_BAD_COORDINATE: Coordinate outside the surface's valid range.
 * @GLITZ_STATUS_NOT_SUPPORTED: Operation not supported (including filters
 * with too many parameters).
 * @GLITZ_STATUS_CONTENT_DESTROYED: The surface is not available anymore,
 * its backing buffer has been deleted.
 *
 * #glitz_status_t defines the possible statuses of a surface,
 * which are returned by #glitz_surface_get_status.
 *
 * Note that @GLITZ_STATUS_NOT_SUPPORTED is not an error, in the sense
 * that it does not (nececssarily) happen because of misuse of the library.
 * Glitz is not meant to handle software fallbacks, and it will return
 * @GLITZ_STATUS_NOT_SUPPORTED when an operation cannot be carried out by
 * graphics hardware.  You should always assume that an operation can fail
 * with @GLITZ_STATUS_NOT_SUPPORTED. */
typedef enum {
  GLITZ_STATUS_SUCCESS = 0,
  GLITZ_STATUS_NO_MEMORY,
  GLITZ_STATUS_BAD_COORDINATE,
  GLITZ_STATUS_NOT_SUPPORTED,
  GLITZ_STATUS_CONTENT_DESTROYED
} glitz_status_t;

const char *
glitz_status_string (glitz_status_t status);


/* glitz_drawable.c */

/**
 * glitz_drawable_buffer_t:
 * @GLITZ_DRAWABLE_BUFFER_FRONT_COLOR: The front buffer of a single- or
 * double-buffered drawable.
 * @GLITZ_DRAWABLE_BUFFER_BACK_COLOR: The back buffer of a double-buffered
 * drawable.
 *
 * A @glitz_drawable_buffer_t is used to specify which side of a drawable
 * is being drawn to, read from, or attached to a surface.
 **/
typedef enum {
  GLITZ_DRAWABLE_BUFFER_FRONT_COLOR,
  GLITZ_DRAWABLE_BUFFER_BACK_COLOR
} glitz_drawable_buffer_t;

glitz_drawable_t *
glitz_create_drawable (glitz_drawable_t        *other,
		       glitz_drawable_format_t *format,
		       unsigned int            width,
		       unsigned int            height);

glitz_drawable_t *
glitz_create_pbuffer_drawable (glitz_drawable_t        *other,
			       glitz_drawable_format_t *format,
			       unsigned int            width,
			       unsigned int            height);

glitz_drawable_t *
glitz_create_pixmap_drawable (glitz_drawable_t        *other,
			      glitz_drawable_format_t *format,
			      unsigned int            width,
			      unsigned int            height);

void
glitz_drawable_destroy (glitz_drawable_t *drawable);

void
glitz_drawable_reference (glitz_drawable_t *drawable);

void
glitz_drawable_update_size (glitz_drawable_t *drawable,
			    unsigned int     width,
			    unsigned int     height);

unsigned int
glitz_drawable_get_width (glitz_drawable_t *drawable);

unsigned int
glitz_drawable_get_height (glitz_drawable_t *drawable);

void
glitz_drawable_swap_buffer_region (glitz_drawable_t *drawable,
				   int              x_origin,
				   int              y_origin,
				   glitz_box_t      *box,
				   int              n_box);

void
glitz_drawable_swap_buffers (glitz_drawable_t *drawable);

void
glitz_drawable_flush (glitz_drawable_t *drawable);

void
glitz_drawable_finish (glitz_drawable_t *drawable);

unsigned long
glitz_drawable_get_features (glitz_drawable_t *drawable);

glitz_drawable_format_t *
glitz_drawable_get_format (glitz_drawable_t *drawable);

typedef enum {
    GLITZ_GL_STRING_VENDOR,
    GLITZ_GL_STRING_RENDERER,
    GLITZ_GL_STRING_VERSION,
    GLITZ_GL_STRING_EXTENSIONS
} glitz_gl_string_t;

const char *
glitz_drawable_get_gl_string (glitz_drawable_t  *drawable,
			      glitz_gl_string_t name);


/* glitz_surface.c */

#define GLITZ_SURFACE_UNNORMALIZED_MASK (1L << 0)

typedef struct _glitz_surface_attributes_t {
  glitz_bool_t unnormalized;
} glitz_surface_attributes_t;

glitz_surface_t *
glitz_surface_create (glitz_drawable_t           *drawable,
		      glitz_format_t             *format,
		      unsigned int               width,
		      unsigned int               height,
		      unsigned long              mask,
		      glitz_surface_attributes_t *attributes);

void
glitz_surface_destroy (glitz_surface_t *surface);

void
glitz_surface_reference (glitz_surface_t *surface);

void
glitz_surface_attach (glitz_surface_t         *surface,
		      glitz_drawable_t        *drawable,
		      glitz_drawable_buffer_t buffer);

void
glitz_surface_detach (glitz_surface_t *surface);

void
glitz_surface_flush (glitz_surface_t *surface);

glitz_drawable_t *
glitz_surface_get_drawable (glitz_surface_t *surface);

glitz_drawable_t *
glitz_surface_get_attached_drawable (glitz_surface_t *surface);

void
glitz_surface_set_transform (glitz_surface_t   *surface,
			     glitz_transform_t *transform);

/**
 * glitz_fill_t:
 * GLITZ_FILL_TRANSPARENT: specifies that areas falling outside a surface
 * are not being painted.
 * GLITZ_FILL_NEAREST: specifies that coordinates falling outside a surface
 * are clamped to the nearest side of the surface.
 * GLITZ_FILL_REPEAT: specifies that a surface is tiled when the user
 * specifies coordinates falling outside it.
 * GLITZ_FILL_REFLECT: specifies that a surface is reflected and tiled when
 * the user specifies coordinates falling outside it.
 **/
typedef enum {
  GLITZ_FILL_TRANSPARENT,
  GLITZ_FILL_NEAREST,
  GLITZ_FILL_REPEAT,
  GLITZ_FILL_REFLECT
} glitz_fill_t;

void
glitz_surface_set_fill (glitz_surface_t *surface,
			glitz_fill_t    fill);

void
glitz_surface_set_component_alpha (glitz_surface_t *surface,
				   glitz_bool_t    component_alpha);

void
glitz_surface_set_filter (glitz_surface_t    *surface,
			  glitz_filter_t     filter,
			  glitz_fixed16_16_t *params,
			  int                n_params);

void
glitz_surface_set_dither (glitz_surface_t *surface,
			  glitz_bool_t    dither);

unsigned int
glitz_surface_get_width (glitz_surface_t *surface);

unsigned int
glitz_surface_get_height (glitz_surface_t *surface);

glitz_status_t
glitz_surface_get_status (glitz_surface_t *surface);

glitz_format_t *
glitz_surface_get_format (glitz_surface_t *surface);

void
glitz_surface_translate_point (glitz_surface_t     *surface,
			       glitz_point_fixed_t *src,
			       glitz_point_fixed_t *dst);

void
glitz_surface_set_clip_region (glitz_surface_t *surface,
			       int             x_origin,
			       int             y_origin,
			       glitz_box_t     *box,
			       int             n_box);

glitz_bool_t
glitz_surface_valid_target (glitz_surface_t *surface);

glitz_bool_t
glitz_surface_bind_tex_image (glitz_surface_t *surface,
                              glitz_drawable_t* drawable);

glitz_bool_t
glitz_surface_release_tex_image (glitz_surface_t *surface,
                                 glitz_drawable_t* drawable);


/* glitz_texture.c */

/**
 * glitz_texture_object_t:
 * 
 * A #glitz_texture_object_t provides functions to use a Glitz
 * surface as the texture map for OpenGL 3D drawing.  Using a
 * Glitz surface just for texturing, without attaching it to
 * a drawable, does not consume any other resources that the
 * texture memory.
 *
 * A #glitz_texture_object_t also stores the texture parameters
 * so that they can be set correctly when the texture is bound.
 **/
typedef struct _glitz_texture_object glitz_texture_object_t;

glitz_texture_object_t *
glitz_texture_object_create (glitz_surface_t *surface);

void
glitz_texture_object_destroy (glitz_texture_object_t *texture);

void
glitz_texture_object_reference (glitz_texture_object_t *texture);

/**
 * glitz_texture_filter_type_t:
 * @GLITZ_TEXTURE_FILTER_TYPE_MAG: Specifies that the
 * texture magnifying filter function is being set.
 * The texture magnifying function is used whenever many destination
 * pixels map to the same texture pixel.
 * @GLITZ_TEXTURE_FILTER_TYPE_MIN: Specifies that the
 * texture minifying filter function is being set.
 * The texture minifying function is used whenever a destination
 * pixel maps to an area greater than one texture pixel.
 *
 * Texture mapping is governed by parameters that determine how
 * samples are derived from the image.  #glitz_texture_filter_type_t
 * defines two of these parameters, whose possible values are
 * #glitz_texture_filter_t.  See also #glitz_texture_wrap_type_t.
 **/
typedef enum {
  GLITZ_TEXTURE_FILTER_TYPE_MAG = 0,
  GLITZ_TEXTURE_FILTER_TYPE_MIN = 1
} glitz_texture_filter_type_t;

/**
 * glitz_texture_filter_t:
 * @GLITZ_TEXTURE_FILTER_NEAREST: Uses the texture pixel that is
 * nearest (in Manhattan distance) to the center of the pixel
 * being textured.
 * @GLITZ_TEXTURE_FILTER_LINEAR: Computes the weighted average of
 * the four texture pixels that are closest to the center of the
 * pixel being textured (bilinear filtering).
 * @GLITZ_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST: Chooses a mipmap
 * that most closely matches the size of the pixel being textured
 * and applies the @GLITZ_TEXTURE_FILTER_NEAREST criterion to it.
 * @GLITZ_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST: Chooses a mipmap
 * that most closely matches the size of the pixel being textured
 * and applies the @GLITZ_TEXTURE_FILTER_LINEAR criterion to it.
 * @GLITZ_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR: Applies the
 * @GLITZ_TEXTURE_FILTER_NEAREST criterion to the two
 * mipmaps that most closely match the size of the pixel being
 * textured, and computes a weighted average between the two.
 * @GLITZ_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: Applies the
 * @GLITZ_TEXTURE_FILTER_LINEAR criterion to the two
 * mipmaps that most closely match the size of the pixel being
 * textured, and computes a weighted average between the two.
 * (trilinear filtering).
 *
 * #glitz_texture_filter_t defines six possible functions
 * used to compute the value of a textured pixel.  The
 * first two apply to both minification and magnification
 * (see #glitz_texture_filter_type_t); the other four only
 * apply to minification.
 **/
typedef enum {
  GLITZ_TEXTURE_FILTER_NEAREST		      = 0,
  GLITZ_TEXTURE_FILTER_LINEAR		      = 1,
  GLITZ_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST = 2,
  GLITZ_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST  = 3,
  GLITZ_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR  = 3,
  GLITZ_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR   = 4
} glitz_texture_filter_t;

void
glitz_texture_object_set_filter (glitz_texture_object_t      *texture,
				 glitz_texture_filter_type_t type,
				 glitz_texture_filter_t      filter);

/**
 * @GLITZ_TEXTURE_WRAP_TYPE_S: Specifies that the wrapping
 * behavior for the s coordinate (the texture x) is being set.
 * @GLITZ_TEXTURE_WRAP_TYPE_T: Specifies that the wrapping
 * behavior for the t coordinate (the texture y) is being set.
 *
 * Texture mapping is governed by parameters that determine how
 * samples are derived from the image.  #glitz_texture_wrap_type_t
 * defines two of these parameters, whose possible values are
 * #glitz_texture_wrap_t.  See also #glitz_texture_filter_type_t.
 **/
typedef enum {
  GLITZ_TEXTURE_WRAP_TYPE_S = 0,
  GLITZ_TEXTURE_WRAP_TYPE_T = 1
} glitz_texture_wrap_type_t;

/**
 * glitz_texture_wrap_t:
 * @GLITZ_TEXTURE_WRAP_CLAMP: Clamp the texture coordinates to [0,1].
 * This causes the border color to be mixed in the texture at its
 * borders, since the texture pixels' centers have a fractional
 * coordinate of 0.5, and is rarely a desired behavior.
 * @GLITZ_TEXTURE_WRAP_CLAMP_TO_EDGE: Clamp the texture coordinates
 * to [0+half texel, 1-half texel].  This is usually the desired
 * behavior.
 * @GLITZ_TEXTURE_WRAP_CLAMP_TO_BORDER: Clamp the texture coordinates
 * to [0-half texel, 1+half texel].  Areas outside the texture
 * are painted in the border color.
 * @GLITZ_TEXTURE_WRAP_REPEAT: Causes the integer part of the texture
 * coordinates to be ignored, thereby creating a repeating pattern.
 * @GLITZ_TEXTURE_WRAP_MIRRORED_REPEAT: Similar to
 * @GLITZ_TEXTURE_WRAP_REPEAT, but it creates a repeating pattern
 * of the texture and its mirror.  The math is boring enough not to
 * be worth explaining it.
 *
 * #glitz_texture_filter_t defines six possible functions
 * used to obtain valid texture coordinates when they are out
 * of range.
 **/
typedef enum {
  GLITZ_TEXTURE_WRAP_CLAMP	     = 0,
  GLITZ_TEXTURE_WRAP_CLAMP_TO_EDGE   = 1,
  GLITZ_TEXTURE_WRAP_CLAMP_TO_BORDER = 2,
  GLITZ_TEXTURE_WRAP_REPEAT	     = 3,
  GLITZ_TEXTURE_WRAP_MIRRORED_REPEAT = 4
} glitz_texture_wrap_t;

void
glitz_texture_object_set_wrap (glitz_texture_object_t    *texture,
			       glitz_texture_wrap_type_t type,
			       glitz_texture_wrap_t      wrap);

void
glitz_texture_object_set_border_color (glitz_texture_object_t *texture,
				       glitz_color_t          *color);

void
glitz_texture_object_pixel_to_texture_coord (glitz_texture_object_t *texture,
					     double		    *u_pixel,
					     double		    *v_pixel);

void
glitz_texture_object_texture_coord_to_pixel (glitz_texture_object_t *texture,
					     double		    *u,
					     double		    *v);

typedef enum {
  GLITZ_TEXTURE_TARGET_2D   = 0,
  GLITZ_TEXTURE_TARGET_RECT = 1
} glitz_texture_target_t;

glitz_texture_target_t
glitz_texture_object_get_target (glitz_texture_object_t *texture);


/* glitz_context.c */

/**
 * glitz_context_t:
 *
 * A #glitz_context_t stores the OpenGL state; OpenGL commands
 * executed while the context is active affect the state and at
 * the same time draw on a drawable.  You only need to create
 * #glitz_context_t to draw with OpenGL commands on other Glitz
 * objects (drawables, as well as surfaces with a drawable attached).
 * Compositing manages contexts autonomously.
 **/
typedef struct _glitz_context glitz_context_t;

glitz_context_t *
glitz_context_create (glitz_drawable_t        *drawable,
		      glitz_drawable_format_t *format);

void
glitz_context_destroy (glitz_context_t *context);

void
glitz_context_reference (glitz_context_t *context);

void
glitz_context_copy (glitz_context_t *src,
		    glitz_context_t *dst,
		    unsigned long   mask);

typedef void (*glitz_lose_current_function_t) (void *closure);

void
glitz_context_set_user_data (glitz_context_t               *context,
			     void                          *closure,
			     glitz_lose_current_function_t lose_current);

typedef void (*glitz_function_pointer_t) (void);

glitz_function_pointer_t
glitz_context_get_proc_address (glitz_context_t *context,
				const char      *name);

void
glitz_context_make_current (glitz_context_t  *context,
			    glitz_drawable_t *drawable);

void
glitz_context_bind_texture (glitz_context_t        *context,
			    glitz_texture_object_t *texture);

void
glitz_context_unbind_texture (glitz_context_t        *context,
			      glitz_texture_object_t *texture);

void
glitz_context_draw_buffers (glitz_context_t	          *context,
			    const glitz_drawable_buffer_t *buffers,
			    int				  n);

void
glitz_context_read_buffer (glitz_context_t		 *context,
			   const glitz_drawable_buffer_t buffer);


/* glitz_rect.c */

void
glitz_set_rectangle (glitz_surface_t     *dst,
		     const glitz_color_t *color,
		     int                 x,
		     int                 y,
		     unsigned int        width,
		     unsigned int        height);

void
glitz_set_rectangles (glitz_surface_t         *dst,
		      const glitz_color_t     *color,
		      const glitz_rectangle_t *rects,
		      int                     n_rects);


/* glitz_buffer.c */

typedef struct _glitz_buffer glitz_buffer_t;

typedef enum {
  GLITZ_BUFFER_HINT_STREAM_DRAW,
  GLITZ_BUFFER_HINT_STREAM_READ,
  GLITZ_BUFFER_HINT_STREAM_COPY,
  GLITZ_BUFFER_HINT_STATIC_DRAW,
  GLITZ_BUFFER_HINT_STATIC_READ,
  GLITZ_BUFFER_HINT_STATIC_COPY,
  GLITZ_BUFFER_HINT_DYNAMIC_DRAW,
  GLITZ_BUFFER_HINT_DYNAMIC_READ,
  GLITZ_BUFFER_HINT_DYNAMIC_COPY
} glitz_buffer_hint_t;

typedef enum {
  GLITZ_BUFFER_ACCESS_READ_ONLY,
  GLITZ_BUFFER_ACCESS_WRITE_ONLY,
  GLITZ_BUFFER_ACCESS_READ_WRITE
} glitz_buffer_access_t;

glitz_buffer_t *
glitz_vertex_buffer_create (glitz_drawable_t    *drawable,
			    void                *data,
			    unsigned int        size,
			    glitz_buffer_hint_t hint);

glitz_buffer_t *
glitz_pixel_buffer_create (glitz_drawable_t    *drawable,
			   void                *data,
			   unsigned int        size,
			   glitz_buffer_hint_t hint);

glitz_buffer_t *
glitz_buffer_create_for_data (void *data);

void
glitz_buffer_destroy (glitz_buffer_t *buffer);

void
glitz_buffer_reference (glitz_buffer_t *buffer);

void
glitz_buffer_set_data (glitz_buffer_t *buffer,
		       int            offset,
		       unsigned int   size,
		       const void     *data);

void
glitz_buffer_get_data (glitz_buffer_t *buffer,
		       int            offset,
		       unsigned int   size,
		       void           *data);

void *
glitz_buffer_map (glitz_buffer_t        *buffer,
		  glitz_buffer_access_t access);

glitz_status_t
glitz_buffer_unmap (glitz_buffer_t *buffer);


/* glitz_pixel.c */

typedef struct _glitz_pixel_masks {
  int           bpp;
  unsigned long alpha_mask;
  unsigned long red_mask;
  unsigned long green_mask;
  unsigned long blue_mask;
} glitz_pixel_masks_t;

typedef struct _glitz_pixel_format {
  glitz_fourcc_t               fourcc;
  glitz_pixel_masks_t          masks;
  int                          xoffset;
  int                          skip_lines;
  int                          bytes_per_line;
  glitz_pixel_scanline_order_t scanline_order;
} glitz_pixel_format_t;

void
glitz_set_pixels (glitz_surface_t      *dst,
		  int                  x_dst,
		  int                  y_dst,
		  int                  width,
		  int                  height,
		  glitz_pixel_format_t *format,
		  glitz_buffer_t       *buffer);

void
glitz_get_pixels (glitz_surface_t      *src,
		  int                  x_src,
		  int                  y_src,
		  int                  width,
		  int                  height,
		  glitz_pixel_format_t *format,
		  glitz_buffer_t       *buffer);


/* glitz_geometry.c */

typedef enum {
  GLITZ_PRIMITIVE_POINTS,
  GLITZ_PRIMITIVE_LINES,
  GLITZ_PRIMITIVE_LINE_STRIP,
  GLITZ_PRIMITIVE_LINE_LOOP,
  GLITZ_PRIMITIVE_TRIANGLES,
  GLITZ_PRIMITIVE_TRIANGLE_STRIP,
  GLITZ_PRIMITIVE_TRIANGLE_FAN,
  GLITZ_PRIMITIVE_QUADS,
  GLITZ_PRIMITIVE_QUAD_STRIP,
  GLITZ_PRIMITIVE_POLYGON
} glitz_primitive_t;

typedef enum {
  GLITZ_DATA_TYPE_SHORT,
  GLITZ_DATA_TYPE_INT,
  GLITZ_DATA_TYPE_FLOAT,
  GLITZ_DATA_TYPE_DOUBLE
} glitz_data_type_t;

typedef enum {
  GLITZ_COORDINATE_SIZE_X,
  GLITZ_COORDINATE_SIZE_XY
} glitz_coordinate_size_t;

typedef struct _glitz_coordinate_attribute {
    glitz_data_type_t       type;
    glitz_coordinate_size_t size;
    int                     offset;
} glitz_coordinate_attribute_t;

#define GLITZ_VERTEX_ATTRIBUTE_SRC_COORD_MASK  (1L << 0)
#define GLITZ_VERTEX_ATTRIBUTE_MASK_COORD_MASK (1L << 1)

typedef struct _glitz_vertex_format {
  glitz_primitive_t            primitive;
  glitz_data_type_t            type;
  unsigned int                 bytes_per_vertex;
  unsigned long                attributes;
  glitz_coordinate_attribute_t src;
  glitz_coordinate_attribute_t mask;
} glitz_vertex_format_t;

typedef struct _glitz_bitmap_format {
  glitz_pixel_scanline_order_t scanline_order;
  unsigned int                 bytes_per_line;
  int                          pad;
} glitz_bitmap_format_t;

typedef enum {
  GLITZ_GEOMETRY_TYPE_NONE,
  GLITZ_GEOMETRY_TYPE_VERTEX,
  GLITZ_GEOMETRY_TYPE_BITMAP
} glitz_geometry_type_t;

typedef union _glitz_geometry_format {
  glitz_vertex_format_t vertex;
  glitz_bitmap_format_t bitmap;
} glitz_geometry_format_t;

void
glitz_set_geometry (glitz_surface_t         *dst,
		    glitz_geometry_type_t   type,
		    glitz_geometry_format_t *format,
		    glitz_buffer_t          *buffer);

void
glitz_set_array (glitz_surface_t    *dst,
		 int                first,
		 int                size,
		 unsigned int       count,
		 glitz_fixed16_16_t x_off,
		 glitz_fixed16_16_t y_off);

typedef struct _glitz_multi_array glitz_multi_array_t;

glitz_multi_array_t *
glitz_multi_array_create (unsigned int size);

void
glitz_multi_array_destroy (glitz_multi_array_t *array);

void
glitz_multi_array_reference (glitz_multi_array_t *array);

void
glitz_multi_array_add (glitz_multi_array_t *array,
		       int                 first,
		       int                 size,
		       unsigned int        count,
		       glitz_fixed16_16_t  x_off,
		       glitz_fixed16_16_t  y_off);

void
glitz_multi_array_reset (glitz_multi_array_t *array);

void
glitz_set_multi_array (glitz_surface_t     *dst,
		       glitz_multi_array_t *array,
		       glitz_fixed16_16_t  x_off,
		       glitz_fixed16_16_t  y_off);


/* glitz_trap.c */

int
glitz_add_trapezoids (glitz_buffer_t    *buffer,
		      int               offset,
		      unsigned int      size,
		      glitz_data_type_t type,
		      glitz_surface_t   *mask,
		      glitz_trapezoid_t *traps,
		      int               n_traps,
		      int               *n_added);

int
glitz_add_traps (glitz_buffer_t    *buffer,
		 int               offset,
		 unsigned int      size,
		 glitz_data_type_t type,
		 glitz_surface_t   *mask,
		 glitz_trap_t      *traps,
		 int               n_traps,
		 int               *n_added);


/* glitz.c */

void
glitz_composite (glitz_operator_t op,
		 glitz_surface_t  *src,
		 glitz_surface_t  *mask,
		 glitz_surface_t  *dst,
		 int              x_src,
		 int              y_src,
		 int              x_mask,
		 int              y_mask,
		 int              x_dst,
		 int              y_dst,
		 int              width,
		 int              height);

void
glitz_copy_area (glitz_surface_t *src,
		 glitz_surface_t *dst,
		 int             x_src,
		 int             y_src,
		 int             width,
		 int             height,
		 int             x_dst,
		 int             y_dst);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* GLITZ_H_INCLUDED */
