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

#include "glitz_aglint.h"

extern glitz_gl_proc_address_list_t _glitz_agl_gl_proc_address;

static glitz_surface_t *
_glitz_agl_surface_create_similar (void *abstract_templ,
                                   glitz_format_name_t format_name,
                                   glitz_bool_t drawable,
                                   int width,
                                   int height);

static void
_glitz_agl_surface_destroy (void *abstract_surface);

static glitz_texture_t *
_glitz_agl_surface_get_texture (void *abstract_surface);

static void
_glitz_agl_surface_update_size (void *abstract_surface);

static void
_glitz_agl_surface_flush (void *abstract_surface);

static glitz_bool_t
_glitz_agl_surface_push_current (void *abstract_surface,
                                 glitz_constraint_t constraint)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;
  glitz_bool_t success = 1;
  
  if (constraint == GLITZ_CN_SURFACE_DRAWABLE_CURRENT &&
      ((!surface->pbuffer) && (!surface->drawable))) {
    constraint = GLITZ_CN_ANY_CONTEXT_CURRENT;
    success = 0;
  }
  
  surface = glitz_agl_context_push_current (surface, constraint);

  if (surface) {
    glitz_surface_setup_environment (&surface->base);
    return 1;
  }

  return success;
}

static void
_glitz_agl_surface_pop_current (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;

  surface = glitz_agl_context_pop_current (surface);
  
  if (surface)
    glitz_surface_setup_environment (&surface->base);
}

static const struct glitz_surface_backend glitz_agl_surface_backend = {
  _glitz_agl_surface_create_similar,
  _glitz_agl_surface_destroy,
  _glitz_agl_surface_push_current,
  _glitz_agl_surface_pop_current,
  _glitz_agl_surface_get_texture,
  _glitz_agl_surface_update_size,
  _glitz_agl_surface_flush
};

static glitz_texture_t *
_glitz_agl_surface_get_texture (void *abstract_surface) {
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;

  if (surface->base.hint_mask & GLITZ_INT_HINT_DIRTY_MASK) {
    if (surface->pbuffer) {
      surface->base.hint_mask &= ~GLITZ_INT_HINT_DIRTY_MASK;
      
      return &surface->base.texture;
    } else 
      glitz_texture_copy_surface (&surface->base.texture, &surface->base,
                                  &surface->base.dirty_region);
    
    surface->base.hint_mask &= ~GLITZ_INT_HINT_DIRTY_MASK;
  }

  if (surface->base.texture.allocated)
    return &surface->base.texture;
  else
    return NULL;
}

static void
_glitz_agl_surface_update_size_for_window (WindowRef window,
                                           int *width,
                                           int *height)
{
  Rect window_bounds;

  GetWindowPortBounds (window, &window_bounds);
  
  *width = window_bounds.right - window_bounds.left;
  *height = window_bounds.bottom - window_bounds.top;
}

static void
_glitz_agl_set_features (glitz_agl_surface_t *surface)
{
  surface->base.feature_mask = surface->thread_info->feature_mask;

  surface->base.feature_mask &= ~GLITZ_FEATURE_CONVOLUTION_FILTER_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_MULTISAMPLE_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK;

  if (surface->thread_info->feature_mask &
      GLITZ_FEATURE_CONVOLUTION_FILTER_MASK)
    surface->base.feature_mask |= GLITZ_FEATURE_CONVOLUTION_FILTER_MASK;

  if (surface->thread_info->feature_mask & GLITZ_FEATURE_MULTISAMPLE_MASK)
    surface->base.feature_mask |= GLITZ_FEATURE_MULTISAMPLE_MASK;
  
  if (surface->thread_info->feature_mask &
      GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK)
    surface->base.feature_mask |= GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK;
}

static glitz_surface_t *
_glitz_agl_surface_create (glitz_agl_thread_info_t *thread_info,
                           glitz_format_t *format,
                           int width,
                           int height)
{
  glitz_agl_surface_t *surface;
  glitz_agl_context_t *context;
  unsigned long texture_mask;

  context = glitz_agl_context_get (thread_info, format, 1);
  if (!context)
    return NULL;

  surface = (glitz_agl_surface_t *) calloc (1, sizeof (glitz_agl_surface_t));
  if (surface == NULL)
    return NULL;

  texture_mask = thread_info->texture_mask;

  /* Seems to be a problem with binding a pbuffer to some power of two sized
     textures. This will try to avoid the problem. */
  if (((width > 1) && (width < 64)) ||
      ((height > 1) && (height < 64))) {
    if (texture_mask != GLITZ_TEXTURE_TARGET_2D_MASK)
      texture_mask &= ~GLITZ_TEXTURE_TARGET_2D_MASK;
  }

  glitz_surface_init (&surface->base,
                      &glitz_agl_surface_backend,
                      &_glitz_agl_gl_proc_address,
                      format,
                      width,
                      height,
                      &thread_info->programs,
                      texture_mask);
  
  surface->thread_info = thread_info;
  surface->context = context;

  surface->base.hint_mask |= GLITZ_HINT_OFFSCREEN_MASK;

  _glitz_agl_set_features (surface);

  if (thread_info->feature_mask & GLITZ_FEATURE_OFFSCREEN_DRAWING_MASK) {
    surface->pbuffer = glitz_agl_pbuffer_create (&surface->base.texture);
    if (surface->pbuffer) {
      glitz_surface_push_current (&surface->base,
                                  GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
      glitz_agl_pbuffer_bind (surface->pbuffer,
                              surface->context->context,
                              &surface->base.texture,
                              surface->base.format);
      glitz_surface_pop_current (&surface->base);
    }
  }

  return &surface->base;
}

glitz_surface_t *
glitz_agl_surface_create (glitz_format_t *format,
                          int width,
                          int height)
{
  return _glitz_agl_surface_create (glitz_agl_thread_info_get (),
                                    format, width, height);
}
slim_hidden_def(glitz_agl_surface_create_offscreen);

glitz_surface_t *
glitz_agl_surface_create_for_window (glitz_format_t *format,
                                     WindowRef window)
{
  glitz_agl_surface_t *surface;
  glitz_agl_context_t *context;
  int width, height;
  glitz_agl_thread_info_t *thread_info = glitz_agl_thread_info_get ();

  context = glitz_agl_context_get (thread_info, format, 0);
  if (!context)
    return NULL;

  _glitz_agl_surface_update_size_for_window (window, &width, &height);

  surface = (glitz_agl_surface_t *) calloc (1, sizeof (glitz_agl_surface_t));
  if (surface == NULL)
    return NULL;

  glitz_surface_init (&surface->base,
                      &glitz_agl_surface_backend,
                      &_glitz_agl_gl_proc_address,
                      format,
                      width,
                      height,
                      &thread_info->programs,
                      thread_info->texture_mask);
  
  surface->thread_info = thread_info;
  surface->context = context;
  surface->window = window;
  surface->drawable = GetWindowPort (window);

  _glitz_agl_set_features (surface);
  
  return &surface->base;
}
slim_hidden_def(glitz_agl_surface_create_for_window);

static glitz_surface_t *
_glitz_agl_surface_create_similar (void *abstract_templ,
                                   glitz_format_name_t format_name,
                                   glitz_bool_t drawable,
                                   int width,
                                   int height)
{
  glitz_agl_surface_t *templ = (glitz_agl_surface_t *) abstract_templ;
  
  if ((!drawable) ||
      (templ->thread_info->agl_feature_mask &
       GLITZ_AGL_FEATURE_PBUFFER_MASK)) {
    glitz_format_t *format;

    format = glitz_format_find_standard (templ->thread_info->formats,
                                         templ->thread_info->n_formats,
                                         GLITZ_FORMAT_OPTION_OFFSCREEN_MASK,
                                         format_name);
    
    if (format)
      return _glitz_agl_surface_create (templ->thread_info, format,
                                        width, height);
  }
      
  return NULL;
}

static void
_glitz_agl_surface_destroy (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;
  AGLContext context = aglGetCurrentContext ();
  
  if (context == surface->context->context) {
    if (surface->pbuffer) {
      AGLPbuffer pbuffer;
      GLuint unused;
      
      aglGetPBuffer (context, &pbuffer, &unused, &unused, &unused);
      
      if (pbuffer == surface->pbuffer)
        glitz_agl_context_make_current (surface);
    } else if (surface->drawable) {
      if (aglGetDrawable (context) == surface->drawable)
        glitz_agl_context_make_current (surface);
    }
  }
  
  if (surface->pbuffer)
    glitz_agl_pbuffer_destroy (surface->pbuffer);
  
  glitz_surface_fini (&surface->base);
  
  free (surface);
}

static void
_glitz_agl_surface_update_size (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;
  
  if (surface->window) {
    _glitz_agl_surface_update_size_for_window (surface->window,
                                               &surface->base.width,
                                               &surface->base.height);
    
    glitz_agl_context_push_current (surface,
                                    GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
    
    aglUpdateContext (surface->context->context);
    
    glitz_agl_context_pop_current (surface);
  }
}

static void
_glitz_agl_surface_flush (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;
  
  if (!surface->window)
    return;

  glitz_agl_context_push_current (surface, GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
  
  aglSwapBuffers (surface->context->context);
  
  glitz_agl_context_pop_current (surface);
}
