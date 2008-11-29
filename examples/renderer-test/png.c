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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <setjmp.h>

static void
transform_argb_data (png_structp png,
		     png_row_infop row_info,
		     png_bytep data)
{
  unsigned int i;

  for (i = 0; i < row_info->rowbytes; i += 4) {
    unsigned char *base = &data[i];
    unsigned char blue = base[0];
    unsigned char green = base[1];
    unsigned char red = base[2];
    unsigned char alpha = base[3];
    int p;

    p = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
    memcpy (base, &p, sizeof (int));
  }
}

static void
user_read_data (png_structp png_ptr,
		png_bytep data, png_size_t length)
{
  unsigned char **buffer = (unsigned char **) png_get_io_ptr (png_ptr);

  memcpy (data, *buffer, length);
  *buffer += length;
}

#define PNG_SIG_SIZE 8

int
render_read_png (unsigned char *buffer,
		 unsigned int *width,
		 unsigned int *height,
		 unsigned char **data)
{
  int i, stride;
  unsigned char png_sig[PNG_SIG_SIZE];
  png_struct *png;
  png_info *info;
  png_uint_32 png_width, png_height;
  int depth, color_type, interlace;
  png_byte **row_pointers;
  unsigned char *b = buffer + PNG_SIG_SIZE;

  memcpy (png_sig, buffer, PNG_SIG_SIZE);
  if (png_check_sig (png_sig, PNG_SIG_SIZE) == 0)
    return 1;

  png = png_create_read_struct (PNG_LIBPNG_VER_STRING,
				NULL, NULL, NULL);
  if (png == NULL)
    return 1;

  info = png_create_info_struct (png);
  if (info == NULL) {
    png_destroy_read_struct (&png, NULL, NULL);
    return 1;
  }

  png_set_read_fn (png, (void *) &b, user_read_data);

  png_set_sig_bytes (png, PNG_SIG_SIZE);

  png_read_info (png, info);

  png_get_IHDR (png, info,
		&png_width, &png_height, &depth,
		&color_type, &interlace, NULL, NULL);

  *width = png_width;
  *height = png_height;

  /* convert palette/gray image to rgb */
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb (png);

  /* expand gray bit depth if needed */
  if (color_type == PNG_COLOR_TYPE_GRAY && depth < 8)
    png_set_gray_1_2_4_to_8 (png);

  /* transform transparency to alpha */
  if (png_get_valid (png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha (png);

  if (depth == 16)
    png_set_strip_16 (png);

  if (depth < 8)
    png_set_packing (png);

  if (color_type == PNG_COLOR_TYPE_GRAY) {
    /* grayscale */
    stride = ((png_width + 3) & -4);
  } else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
    /* grayscale with alpha */
    stride = ((2 * png_width + 3) & -4);
  } else {
    /* rgba */
    png_set_read_user_transform_fn (png, transform_argb_data);
    png_set_bgr (png);
    stride = 4 * png_width;
  }

  if (interlace != PNG_INTERLACE_NONE)
    png_set_interlace_handling (png);

  png_read_update_info (png, info);

  *data = malloc (stride * png_height);
  if (*data == NULL)
    return 1;

  row_pointers = (png_byte **) malloc (png_height * sizeof (char *));
  for (i = 0; i < png_height; i++)
    row_pointers[i] = (png_byte *) (*data + i * stride);

  png_read_image (png, row_pointers);
  png_read_end (png, info);

  free (row_pointers);

  png_destroy_read_struct (&png, &info, NULL);

  return 0;
}
