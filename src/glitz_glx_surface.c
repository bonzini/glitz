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

static glitz_surface_t *
_glitz_glx_surface_create_similar (void *abstract_templ,
                                   glitz_format_name_t format_name,
                                   glitz_bool_t drawable,
                                   int width,
                                   int height);

static void
_glitz_glx_surface_destroy (void *abstract_surface);

static glitz_texture_t *
_glitz_glx_surface_get_texture (void *abstract_surface);

static void
_glitz_glx_surface_update_size (void *abstract_surface);

static void
_glitz_glx_surface_flush (void *abstract_surface);

static glitz_bool_t
_glitz_glx_surface_push_current (void *abstract_surface,
                                 glitz_constraint_t constraint)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;

  if (constraint == GLITZ_CN_SURFACE_DRAWABLE_CURRENT) {
    if (surface->render_texture) {
      surface->context->glx.release_tex_image_ati
        (surface->screen_info->display_info->display, surface->pbuffer,
         (surface->base.format->doublebuffer)?
         GLX_BACK_LEFT_ATI: GLX_FRONT_LEFT_ATI);
    }
    if (!surface->drawable)
      constraint = GLITZ_CN_ANY_CONTEXT_CURRENT;
  }
  
  surface = glitz_glx_context_push_current (surface, constraint);

  if (surface) {
    glitz_surface_setup_environment (&surface->base);
    return 1;
  }

  return 0;
}

static void
_glitz_glx_surface_pop_current (void *abstract_surface)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;

  surface = glitz_glx_context_pop_current (surface);
  
  if (surface)
    glitz_surface_setup_environment (&surface->base);
}

static const struct glitz_surface_backend glitz_glx_surface_backend = {
  _glitz_glx_surface_create_similar,
  _glitz_glx_surface_destroy,
  _glitz_glx_surface_push_current,
  _glitz_glx_surface_pop_current,
  _glitz_glx_surface_get_texture,
  _glitz_glx_surface_update_size,
  _glitz_glx_surface_flush
};

static glitz_bool_t
_glitz_glx_surface_update_size_for_window (Display *display,
                                           Window drawable,
                                           int *width,
                                           int *height)
{
  unsigned int uwidth, uheight, bwidth_ignore, depth_ignore;
  Window root_ignore;
  int x_ignore, y_ignore;
  
  if (!XGetGeometry (display, drawable, &root_ignore, &x_ignore, &y_ignore,
                     &uwidth, &uheight, &bwidth_ignore, &depth_ignore))
    return 0;
  
  *width = (int) uwidth;
  *height = (int) uheight;
  
  return 1;
}

static void
_glitz_glx_surface_ensure_texture (glitz_glx_surface_t *surface)
{
  if (!(surface->base.hint_mask & GLITZ_INT_HINT_DIRTY_MASK))
    return;

  if (surface->render_texture) {
    glitz_texture_bind (surface->base.gl, surface->base.texture);
    surface->context->glx.bind_tex_image_ati
      (surface->screen_info->display_info->display, surface->pbuffer,
       (surface->base.format->doublebuffer)?
       GLX_BACK_LEFT_ATI: GLX_FRONT_LEFT_ATI);
    
    glitz_texture_unbind (surface->base.gl, surface->base.texture);

  } else
    glitz_texture_copy_surface (surface->base.texture, &surface->base,
                                &surface->base.dirty_region);

  surface->base.hint_mask &= ~GLITZ_INT_HINT_DIRTY_MASK;
}

static glitz_texture_t *
_glitz_glx_surface_get_texture (void *abstract_surface)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;

  if (!surface->base.texture->allocated)
    glitz_texture_allocate (surface->base.gl, surface->base.texture);
  
  _glitz_glx_surface_ensure_texture (surface);
  
  return surface->base.texture;
}

static void
_glitz_glx_set_features (glitz_glx_surface_t *surface)
{
  surface->base.feature_mask = surface->screen_info->feature_mask;

  surface->base.feature_mask &= ~GLITZ_FEATURE_ATI_RENDER_TEXTURE_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_ARB_VERTEX_PROGRAM_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_CONVOLUTION_FILTER_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_MULTISAMPLE_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK;

  if (surface->context->glx.supported) {
    glitz_surface_push_current (&surface->base,
                                GLITZ_CN_SURFACE_CONTEXT_CURRENT);
    glitz_surface_pop_current (&surface->base);
  }
  
  if ((surface->screen_info->feature_mask &
       GLITZ_FEATURE_ATI_RENDER_TEXTURE_MASK) &&
      surface->context->glx.bind_tex_image_ati &&
      surface->context->glx.release_tex_image_ati)
    surface->base.feature_mask |= GLITZ_FEATURE_ATI_RENDER_TEXTURE_MASK;

  if (surface->context->gl.active_texture_arb &&
      surface->context->gl.multi_tex_coord_2d_arb &&
      surface->context->gl.gen_programs_arb &&
      surface->context->gl.delete_programs_arb &&
      surface->context->gl.program_string_arb &&
      surface->context->gl.bind_program_arb) {
    if (surface->screen_info->feature_mask &
        GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK)
      surface->base.feature_mask |= GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK;
  
    if (surface->screen_info->feature_mask &
        GLITZ_FEATURE_ARB_VERTEX_PROGRAM_MASK)
      surface->base.feature_mask |= GLITZ_FEATURE_ARB_VERTEX_PROGRAM_MASK;
    
    if ((surface->base.feature_mask &
         GLITZ_FEATURE_ARB_VERTEX_PROGRAM_MASK) &&
        (surface->base.feature_mask &
         GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK) &&
        surface->context->gl.program_local_param_4d_arb &&
        surface->context->gl.get_program_iv_arb) {
      glitz_gl_uint_t texture_indirections;

      surface->context->gl.get_program_iv_arb
        (GLITZ_GL_FRAGMENT_PROGRAM_ARB,
         GLITZ_GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB,
         &texture_indirections);
      
      /* Convolution filter programs require support for at least six
         texture indirections. */
      if (texture_indirections >= 5)
        surface->base.feature_mask |= GLITZ_FEATURE_CONVOLUTION_FILTER_MASK;
    }
  }

  if (surface->base.format->multisample.supported) {
    surface->base.feature_mask |= GLITZ_FEATURE_MULTISAMPLE_MASK;
    if (surface->screen_info->feature_mask &
        GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK)
      surface->base.feature_mask |= GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK;
  }
}

static glitz_surface_t *
_glitz_glx_surface_create (glitz_glx_screen_info_t *screen_info,
                           glitz_format_t *format,
                           int width,
                           int height)
{
  glitz_glx_surface_t *surface;
  glitz_glx_context_t *context;
  unsigned int texture_format;

  context = glitz_glx_context_get (screen_info, format);
  if (!context)
    return NULL;
  
  surface = (glitz_glx_surface_t *) calloc (1, sizeof (glitz_glx_surface_t));
  if (surface == NULL)
    return NULL;

  glitz_surface_init (&surface->base, &glitz_glx_surface_backend);

  surface->screen_info = screen_info;
  surface->context = context;
  
  surface->base.programs = &screen_info->programs;
  surface->base.format = format;
  surface->base.width = width;
  surface->base.height = height;
  surface->base.hint_mask |= GLITZ_HINT_OFFSCREEN_MASK;
  surface->base.gl = &context->gl;

  texture_format = glitz_get_gl_format_from_bpp (format->bpp);

  if (screen_info->feature_mask & GLITZ_FEATURE_ATI_RENDER_TEXTURE_MASK)
    surface->render_texture = 1;
  
  glitz_surface_push_current (&surface->base, GLITZ_CN_ANY_CONTEXT_CURRENT);

  surface->base.texture =
    glitz_texture_generate (surface->base.gl,
                            width, height,
                            texture_format,
                            screen_info->texture_mask);

  if (!surface->base.texture) {
    glitz_surface_destroy (&surface->base);
    return NULL;
  }

  if (screen_info->feature_mask & GLITZ_FEATURE_OFFSCREEN_DRAWING_MASK)
    surface->drawable = surface->pbuffer =
      glitz_glx_pbuffer_create (screen_info->display_info->display,
                                surface->context->fbconfig,
                                surface->base.texture,
                                surface->render_texture);

  _glitz_glx_set_features (surface);

  if (surface->base.feature_mask & GLITZ_FEATURE_ATI_RENDER_TEXTURE_MASK) {
    if (format->red_size || format->green_size || format->blue_size)
      surface->base.hint_mask |= GLITZ_INT_HINT_REQUIRES_NO_FLIPPING_MASK;
  } else
    surface->render_texture = 0;
  
  if ((!surface->render_texture) && (!surface->pbuffer))
    glitz_texture_allocate (surface->base.gl, surface->base.texture);

  glitz_surface_pop_current (&surface->base);

  return &surface->base;
}

glitz_surface_t *
glitz_glx_surface_create (Display *display,
                          int screen,
                          glitz_format_t *format,
                          int width,
                          int height)
{

  return
    _glitz_glx_surface_create (glitz_glx_screen_info_get
                               (display, screen),
                               format, width, height);
}
slim_hidden_def(glitz_glx_surface_create);

glitz_surface_t *
glitz_glx_surface_create_for_window (Display *display,
                                     int screen,
                                     glitz_format_t *format,
                                     Window window)
{
  glitz_glx_surface_t *surface;
  glitz_glx_context_t *context;
  int width, height;
  glitz_glx_screen_info_t *screen_info =
    glitz_glx_screen_info_get (display, screen);
  
  context = glitz_glx_context_get (screen_info, format);
  if (!context)
    return NULL;

  if (!_glitz_glx_surface_update_size_for_window (display, window,
                                                  &width, &height))
    return NULL;

  surface = (glitz_glx_surface_t *) calloc (1, sizeof (glitz_glx_surface_t));
  if (surface == NULL)
    return NULL;

  glitz_surface_init (&surface->base, &glitz_glx_surface_backend);
  
  surface->screen_info = screen_info;
  surface->context = context;

  surface->base.programs = &screen_info->programs;
  surface->base.format = format;
  surface->base.width = width;
  surface->base.height = height;
  surface->base.gl = &context->gl;
  
  surface->drawable = window;

  _glitz_glx_set_features (surface);
  
  return &surface->base;
}
slim_hidden_def(glitz_glx_surface_create_for_window);

static glitz_surface_t *
_glitz_glx_surface_create_similar (void *abstract_templ,
                                   glitz_format_name_t format_name,
                                   glitz_bool_t drawable,
                                   int width,
                                   int height)
{
  glitz_glx_surface_t *templ = (glitz_glx_surface_t *) abstract_templ;
  
  if ((!drawable) || (templ->screen_info->feature_mask &
                      GLITZ_FEATURE_OFFSCREEN_DRAWING_MASK)) {
    glitz_format_t *format;
    
    format = glitz_format_find_sufficient_standard
      (templ->base.format, 1, GLITZ_FORMAT_OPTION_OFFSCREEN_MASK, format_name);

    if (!format)
      format = glitz_format_find_standard (templ->screen_info->formats,
                                           templ->screen_info->n_formats,
                                           GLITZ_FORMAT_OPTION_OFFSCREEN_MASK,
                                           format_name);
    if (format)
      return _glitz_glx_surface_create (templ->screen_info, format,
                                        width, height);
  }
  
  return NULL;
}

static void
_glitz_glx_surface_destroy (void *abstract_surface)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;

  glitz_surface_push_current (&surface->base, GLITZ_CN_ANY_CONTEXT_CURRENT);

  if (surface->render_texture) {
    surface->context->glx.release_tex_image_ati
      (surface->screen_info->display_info->display, surface->pbuffer,
       (surface->base.format->doublebuffer)?
       GLX_BACK_LEFT_ATI: GLX_FRONT_LEFT_ATI);
  }
  
  if (surface->base.texture)
    glitz_texture_destroy (surface->base.gl, surface->base.texture);

  if (surface->pbuffer)
    glitz_glx_pbuffer_destroy (surface->screen_info->display_info->display,
                               surface->pbuffer);
  
  glitz_surface_pop_current (&surface->base);

  if (glXGetCurrentDrawable () == surface->drawable) {
    surface->drawable = None;
    glitz_glx_context_make_current (surface);
  }
  
  glitz_surface_deinit (&surface->base);
  
  free (surface);
}


static void
_glitz_glx_surface_update_size (void *abstract_surface)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;
  
  if ((! surface->pbuffer) && surface->drawable) {
    _glitz_glx_surface_update_size_for_window
      (surface->screen_info->display_info->display, surface->drawable,
       &surface->base.width, &surface->base.height);
  }
}

static void
_glitz_glx_surface_flush (void *abstract_surface)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;

  if (surface->pbuffer || (!surface->drawable))
    return;

  glitz_glx_context_push_current (surface, GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
  
  glXSwapBuffers (surface->screen_info->display_info->display,
                  surface->drawable);
  
  glitz_glx_context_pop_current (surface);
}
