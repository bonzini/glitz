/*
 * Copyright � 2004 David Reveman, Peter Nilsson
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

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitz_glxint.h"

GLXPbuffer
glitz_glx_pbuffer_create (glitz_glx_display_info_t *display_info,
                          GLXFBConfig fbconfig,
                          glitz_texture_t *texture)
{
  int pbuffer_attr[13], i = 0;

  pbuffer_attr[i++] = GLX_PBUFFER_WIDTH;
  pbuffer_attr[i++] = texture->width;
  pbuffer_attr[i++] = GLX_PBUFFER_HEIGHT;
  pbuffer_attr[i++] = texture->height;
  pbuffer_attr[i++] = GLX_PRESERVED_CONTENTS;
  pbuffer_attr[i++] = 1;
  pbuffer_attr[i++] = GLX_LARGEST_PBUFFER;
  pbuffer_attr[i++] = 0;
  pbuffer_attr[i++] = 0;
    
  return
    display_info->thread_info->glx.create_pbuffer (display_info->display,
                                                   fbconfig, pbuffer_attr);
}

void 
glitz_glx_pbuffer_destroy (glitz_glx_display_info_t *display_info,
                           GLXPbuffer pbuffer)
{
  display_info->thread_info->glx.destroy_pbuffer (display_info->display,
                                                  pbuffer);
}
