/*
 * Copyright � 2004 David Reveman
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
 * Author: David Reveman <c99drn@cs.umu.se>
 */

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitzint.h"

#define STORE_16(dst, size, src) \
  dst = ((size) ? \
         ((((((1L << (size)) - 1L) * (src)) / 0xffff) * 0xffff) / \
          ((1L << (size)) - 1L)) : \
         dst)

static void
glitz_rectangle_bounds (const glitz_rectangle_t *rects,
                        int                     n_rects,
                        glitz_box_t             *box)
{
  box->x1 = MAXSHORT;
  box->x2 = MINSHORT;
  box->y1 = MAXSHORT;
  box->y2 = MINSHORT;
  
  for (; n_rects; n_rects--, rects++) {
    if (rects->x < box->x1)
      box->x1 = rects->x;
    if ((rects->x + rects->width) > box->x2)
      box->x2 = rects->x + rects->width;
    if (rects->y < box->y1)
      box->y1 = rects->y;
    if ((rects->y + rects->height) > box->y2)
      box->y2 = rects->y + rects->height;
  }
}

void
glitz_set_rectangles (glitz_surface_t         *dst,
                      const glitz_color_t     *color,
                      const glitz_rectangle_t *rects,
                      int                     n_rects)
{
  glitz_box_t bounds;

  GLITZ_GL_SURFACE (dst);

  glitz_rectangle_bounds (rects, n_rects, &bounds);
  if (bounds.x1 > dst->width || bounds.y1 > dst->height ||
      bounds.x2 < 0 || bounds.y2 < 0)
    return;

  if (SURFACE_SOLID (dst) &&
      (bounds.x2 - bounds.x1) > 0 && (bounds.y2 - bounds.y1) > 0) {
    STORE_16 (dst->solid.red, dst->format->color.red_size, color->red);
    STORE_16 (dst->solid.green, dst->format->color.green_size, color->green);
    STORE_16 (dst->solid.blue, dst->format->color.blue_size, color->blue);
    STORE_16 (dst->solid.alpha, dst->format->color.alpha_size, color->alpha);

    glitz_surface_damage (dst, &bounds,
                          GLITZ_DAMAGE_TEXTURE_MASK |
                          GLITZ_DAMAGE_DRAWABLE_MASK);
    return;
  }

  if (glitz_surface_push_current (dst, GLITZ_DRAWABLE_CURRENT)) {
    gl->clear_color (color->red / (glitz_gl_clampf_t) 0xffff,
                     color->green / (glitz_gl_clampf_t) 0xffff,
                     color->blue / (glitz_gl_clampf_t) 0xffff,
                     color->alpha / (glitz_gl_clampf_t) 0xffff);

    for (; n_rects; n_rects--, rects++) {
      gl->scissor (rects->x,
                   dst->attached->height - dst->y - rects->y - rects->height,
                   rects->width,
                   rects->height);
      gl->clear (GLITZ_GL_COLOR_BUFFER_BIT);
    }
    glitz_surface_damage (dst, &bounds,
                          GLITZ_DAMAGE_TEXTURE_MASK |
                          GLITZ_DAMAGE_SOLID_MASK);
  } else {
    static glitz_pixel_format_t pf = {
      {
        32,
        0xff000000,
        0x00ff0000,
        0x0000ff00,
        0x000000ff
      },
      0, 0, 0,
      GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP
    };
    glitz_buffer_t *buffer;
    unsigned int i, width, height, pixel, *data;

    width = bounds.x2 - bounds.x1;
    height = bounds.y2 - bounds.y1;

    data = malloc (width * height * 4);
    if (data == NULL) {
      glitz_surface_status_add (dst, GLITZ_STATUS_NO_MEMORY_MASK);
      return;
    }
    
    buffer = glitz_buffer_create_for_data (data);
    if (buffer == NULL) {
      free (data);
      glitz_surface_status_add (dst, GLITZ_STATUS_NO_MEMORY_MASK);
      return;
    }        
    
    pixel =
      ((((unsigned int) color->alpha * 0xff) / 0xffff) << 24) |
      ((((unsigned int) color->red * 0xff) / 0xffff) << 16) |
      ((((unsigned int) color->green * 0xff) / 0xffff) << 8) |
      ((((unsigned int) color->blue * 0xff) / 0xffff));
    
    for (i = 0; i < width; i++)
      data[i] = pixel;
    
    for (i = 1; i < height; i++)
      memcpy (&data[i * width], data, width * sizeof (unsigned int));

    for (; n_rects; n_rects--, rects++)
      glitz_set_pixels (dst,
                        rects->x, rects->y,
                        rects->width, rects->height,
                        &pf, buffer);

    glitz_buffer_destroy (buffer);
    free (data);
  }
  
  glitz_surface_pop_current (dst);
}
slim_hidden_def(glitz_set_rectangles);

void
glitz_set_rectangle (glitz_surface_t     *dst,
                     const glitz_color_t *color,
                     int                 x,
                     int                 y,
                     unsigned int        width,
                     unsigned int        height)
{
  glitz_rectangle_t rect;

  rect.x = x;
  rect.y = y;
  rect.width = width;
  rect.height = height;

  glitz_set_rectangles (dst, color, &rect, 1);
}
slim_hidden_def(glitz_set_rectangle);
