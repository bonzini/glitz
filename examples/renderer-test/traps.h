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
 * Author: David Reveman <davidr@freedesktop.org>
 */

#ifndef TRAPS_H_INCLUDED
#define TRAPS_H_INCLUDED

render_trapezoid_t curve_rectangle_traps[] = {
  {
    { 0x006c0000, 0x006c0000, 0x00059999 },
    { 0x0059f998, 0x007e0665, 0x00059fff }
  }, {
    { 0x0059f998, 0x007e0665, 0x00059fff },
    { 0x004a3332, 0x008dcccb, 0x0005cccc }
  }, {
    { 0x004a3332, 0x008dcccb, 0x0005cccc },
    { 0x003c8664, 0x009b7997, 0x00064664 }
  }, {
    { 0x003c8664, 0x009b7997, 0x00064664 },
    { 0x00366d97, 0x00a19264, 0x0006abfd }
  }, {
    { 0x00366d97, 0x00a19264, 0x0006abfd },
    { 0x0030cccb, 0x00a73332, 0x00073332 }
  }, {
    { 0x0030cccb, 0x00a73332, 0x00073332 },
    { 0x002b9f30, 0x00ac60ca, 0x0007e0cb }
  }, {
    { 0x002b9f30, 0x00ac60ca, 0x0007e0cb },
    { 0x0026dffe, 0x00b11ffe, 0x0008b998 }
  }, {
    { 0x0026dffe, 0x00b11ffe, 0x0008b998 },
    { 0x00228a64, 0x00b57598, 0x0009c264 }
  }, {
    { 0x00228a64, 0x00b57598, 0x0009c264 },
    { 0x001e9998, 0x00b96665, 0x000afffe }
  }, {
    { 0x001e9998, 0x00b96665, 0x000afffe },
    { 0x001b08cb, 0x00bcf731, 0x000c7730 }
  }, {
    { 0x001b08cb, 0x00bcf731, 0x000c7730 },
    { 0x0017d331, 0x00c02ccb, 0x000e2cca }
  }, {
    { 0x0017d331, 0x00c02ccb, 0x000e2cca },
    { 0x0014f3fd, 0x00c30bfe, 0x00102597 }
  }, {
    { 0x0014f3fd, 0x00c30bfe, 0x00102597 },
    { 0x00126665, 0x00c59999, 0x00126665 }
  }, {
    { 0x00126665, 0x00c59999, 0x00126665 },
    { 0x00102597, 0x00c7da64, 0x0014f3fd }
  }, {
    { 0x00102597, 0x00c7da64, 0x0014f3fd },
    { 0x000e2cca, 0x00c9d331, 0x0017d331 }
  }, {
    { 0x000e2cca, 0x00c9d331, 0x0017d331 },
    { 0x000c7730, 0x00cb88ca, 0x001b08cb }
  }, {
    { 0x000c7730, 0x00cb88ca, 0x001b08cb },
    { 0x000afffe, 0x00ccfffe, 0x001e9998 }
  }, {
    { 0x000afffe, 0x00ccfffe, 0x001e9998 },
    { 0x0009c264, 0x00ce3d97, 0x00228a64 }
  }, {
    { 0x0009c264, 0x00ce3d97, 0x00228a64 },
    { 0x0008b998, 0x00cf4664, 0x0026dffe }
  }, {
    { 0x0008b998, 0x00cf4664, 0x0026dffe },
    { 0x0007e0cb, 0x00d01f31, 0x002b9f30 }
  }, {
    { 0x0007e0cb, 0x00d01f31, 0x002b9f30 },
    { 0x00073332, 0x00d0cccc, 0x0030cccb }
  }, {
    { 0x00073332, 0x00d0cccc, 0x0030cccb },
    { 0x0006abfd, 0x00d153fe, 0x00366d97 }
  }, {
    { 0x0006abfd, 0x00d153fe, 0x00366d97 },
    { 0x00064664, 0x00d1b998, 0x003c8664 }
  }, {
    { 0x00064664, 0x00d1b998, 0x003c8664 },
    { 0x0005cccc, 0x00d23332, 0x004a3332 }
  }, {
    { 0x0005cccc, 0x00d23332, 0x004a3332 },
    { 0x00059fff, 0x00d25fff, 0x0059f998 }
  }, {
    { 0x00059fff, 0x00d25fff, 0x0059f998 },
    { 0x00059999, 0x00d26666, 0x006c0000 }
  }, {
    { 0x00059999, 0x00d26666, 0x006c0000 },
    { 0x00059fff, 0x00d26000, 0x007e0665 }
  }, {
    { 0x00059fff, 0x00d25fff, 0x007e0665 },
    { 0x0005cccc, 0x00d23333, 0x008dcccb }
  }, {
    { 0x0005cccc, 0x00d23332, 0x008dcccb },
    { 0x00064664, 0x00d1b999, 0x009b7997 }
  }, {
    { 0x00064664, 0x00d1b998, 0x009b7997 },
    { 0x0006abfd, 0x00d153ff, 0x00a19264 }
  }, {
    { 0x0006abfd, 0x00d153fe, 0x00a19264 },
    { 0x00073332, 0x00d0cccd, 0x00a73332 }
  }, {
    { 0x00073332, 0x00d0cccc, 0x00a73332 },
    { 0x0007e0cb, 0x00d01f32, 0x00ac60ca }
  }, {
    { 0x0007e0cb, 0x00d01f31, 0x00ac60ca },
    { 0x0008b998, 0x00cf4665, 0x00b11ffe }
  }, {
    { 0x0008b998, 0x00cf4664, 0x00b11ffe },
    { 0x0009c264, 0x00ce3d98, 0x00b57598 }
  }, {
    { 0x0009c264, 0x00ce3d97, 0x00b57598 },
    { 0x000afffe, 0x00ccffff, 0x00b96665 }
  }, {
    { 0x000afffe, 0x00ccfffe, 0x00b96665 },
    { 0x000c7730, 0x00cb88cb, 0x00bcf731 }
  }, {
    { 0x000c7730, 0x00cb88ca, 0x00bcf731 },
    { 0x000e2cca, 0x00c9d332, 0x00c02ccb }
  }, {
    { 0x000e2cca, 0x00c9d331, 0x00c02ccb },
    { 0x00102597, 0x00c7da65, 0x00c30bfe }
  }, {
    { 0x00102597, 0x00c7da64, 0x00c30bfe },
    { 0x00126665, 0x00c5999a, 0x00c59999 }
  }, {
    { 0x00126665, 0x00c59999, 0x00c59999 },
    { 0x0014f3fd, 0x00c30bff, 0x00c7da64 }
  }, {
    { 0x0014f3fd, 0x00c30bfe, 0x00c7da64 },
    { 0x0017d331, 0x00c02ccc, 0x00c9d331 }
  }, {
    { 0x0017d331, 0x00c02ccb, 0x00c9d331 },
    { 0x001b08cb, 0x00bcf732, 0x00cb88ca }
  }, {
    { 0x001b08cb, 0x00bcf731, 0x00cb88ca },
    { 0x001e9998, 0x00b96666, 0x00ccfffe }
  }, {
    { 0x001e9998, 0x00b96665, 0x00ccfffe },
    { 0x00228a64, 0x00b57599, 0x00ce3d97 }
  }, {
    { 0x00228a64, 0x00b57598, 0x00ce3d97 },
    { 0x0026dffe, 0x00b11fff, 0x00cf4664 }
  }, {
    { 0x0026dffe, 0x00b11ffe, 0x00cf4664 },
    { 0x002b9f30, 0x00ac60cb, 0x00d01f31 }
  }, {
    { 0x002b9f30, 0x00ac60ca, 0x00d01f31 },
    { 0x0030cccb, 0x00a73333, 0x00d0cccc }
  }, {
    { 0x0030cccb, 0x00a73332, 0x00d0cccc },
    { 0x00366d97, 0x00a19265, 0x00d153fe }
  }, {
    { 0x00366d97, 0x00a19264, 0x00d153fe },
    { 0x003c8664, 0x009b7998, 0x00d1b998 }
  }, {
    { 0x003c8664, 0x009b7997, 0x00d1b998 },
    { 0x004a3332, 0x008dcccc, 0x00d23332 }
  }, {
    { 0x004a3332, 0x008dcccb, 0x00d23332 },
    { 0x0059f998, 0x007e0666, 0x00d25fff }
  }, {
    { 0x0059f998, 0x007e0665, 0x00d25fff },
    { 0x006c0000, 0x006c0001, 0x00d26666 }
  }
};

#endif /* TRAPS_H_INCLUDED */