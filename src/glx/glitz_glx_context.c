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

#include "glitz_glxint.h"

#include <stdlib.h>

extern glitz_gl_proc_address_list_t _glitz_glx_gl_proc_address;

static void
_glitz_glx_context_create (glitz_glx_screen_info_t *screen_info,
                           XID                     visualid,
                           GLXContext              share_list,
                           glitz_glx_context_t     *context)
{
  int vis_info_count, i;
  XVisualInfo *vis_infos;

  vis_infos = XGetVisualInfo (screen_info->display_info->display,
                              0, NULL, &vis_info_count);
  for (i = 0; i < vis_info_count; i++) {
    if (vis_infos[i].visual->visualid == visualid)
      break;
  }

  context->context = glXCreateContext (screen_info->display_info->display,
                                       &vis_infos[i], share_list,
                                       GLITZ_GL_TRUE);
  context->id = visualid;  
  context->fbconfig = (GLXFBConfig) 0;

  XFree (vis_infos);
}

static void
_glitz_glx_context_create_using_fbconfig (glitz_glx_screen_info_t *screen_info,
                                          XID                     fbconfigid,
                                          GLXContext              share_list,
                                          glitz_glx_context_t     *context)
{
  GLXFBConfig *fbconfigs;
  int i, n_fbconfigs;
  XVisualInfo *vinfo = NULL;
  glitz_glx_static_proc_address_list_t *glx = &screen_info->glx;

  fbconfigs = glx->get_fbconfigs (screen_info->display_info->display,
                                  screen_info->screen, &n_fbconfigs);
  for (i = 0; i < n_fbconfigs; i++) {
    int value;
    
    glx->get_fbconfig_attrib (screen_info->display_info->display, fbconfigs[i],
                              GLX_FBCONFIG_ID, &value);
    if (value == (int) fbconfigid)
      break;
  }

  if (i < n_fbconfigs)
    vinfo = glx->get_visual_from_fbconfig (screen_info->display_info->display,
                                           fbconfigs[i]);

  context->id = fbconfigid;
  if (vinfo) {
    context->context = glXCreateContext (screen_info->display_info->display,
                                         vinfo, share_list, GLITZ_GL_TRUE);
    XFree (vinfo);
  } else if (glx->create_new_context)
    context->context =
      glx->create_new_context (screen_info->display_info->display,
                               fbconfigs[i], GLX_RGBA_TYPE, share_list,
                               GLITZ_GL_TRUE);
  
  if (context->context)
    context->fbconfig = fbconfigs[i];
  else
    context->fbconfig = (GLXFBConfig) 0;

  if (fbconfigs)
    XFree (fbconfigs);
}

glitz_glx_context_t *
glitz_glx_context_get (glitz_glx_screen_info_t *screen_info,
                       glitz_drawable_format_t *format)
{
  glitz_glx_context_t *context;
  glitz_glx_context_t **contexts = screen_info->contexts;
  int index, n_contexts = screen_info->n_contexts;
  XID format_id;

  for (; n_contexts; n_contexts--, contexts++)
    if ((*contexts)->id == screen_info->format_ids[format->id])
      return *contexts;

  index = screen_info->n_contexts++;

  screen_info->contexts =
    realloc (screen_info->contexts,
             sizeof (glitz_glx_context_t *) * screen_info->n_contexts);
  if (!screen_info->contexts)
    return NULL;

  context = malloc (sizeof (glitz_glx_context_t));
  if (!context)
    return NULL;
  
  screen_info->contexts[index] = context;

  format_id = screen_info->format_ids[format->id];

  if (screen_info->glx_feature_mask & GLITZ_GLX_FEATURE_FBCONFIG_MASK)
    _glitz_glx_context_create_using_fbconfig (screen_info,
                                              format_id,
                                              screen_info->root_context,
                                              context);
  else
    _glitz_glx_context_create (screen_info,
                               format_id,
                               screen_info->root_context,
                               context);

  if (!screen_info->root_context)
    screen_info->root_context = context->context;

  memcpy (&context->backend.gl,
          &_glitz_glx_gl_proc_address,
          sizeof (glitz_gl_proc_address_list_t));

  context->backend.create_pbuffer = glitz_glx_create_pbuffer;
  context->backend.destroy = glitz_glx_destroy;
  context->backend.push_current = glitz_glx_push_current;
  context->backend.pop_current = glitz_glx_pop_current;
  context->backend.swap_buffers = glitz_glx_swap_buffers;
  context->backend.make_current_read = glitz_glx_make_current_read;

  context->backend.drawable_formats = screen_info->formats;
  context->backend.n_drawable_formats = screen_info->n_formats;

  context->backend.texture_formats = NULL;
  context->backend.formats = NULL;
  context->backend.n_formats = 0;
  
  context->backend.program_map = &screen_info->program_map;
  context->backend.feature_mask = 0;

  context->initialized = 0;
  
  return context;
}

void
glitz_glx_context_destroy (glitz_glx_screen_info_t *screen_info,
                           glitz_glx_context_t     *context)
{
  if (context->backend.formats)
    free (context->backend.formats);

  if (context->backend.texture_formats)
    free (context->backend.texture_formats);
  
  glXDestroyContext (screen_info->display_info->display,
                     context->context);
  free (context);
}

static void
_glitz_glx_context_initialize (glitz_glx_screen_info_t *screen_info,
                               glitz_glx_context_t     *context)
{
  glitz_backend_init (&context->backend,
                      glitz_glx_get_proc_address,
                      (void *) screen_info);

  context->backend.gl.get_integer_v (GLITZ_GL_MAX_VIEWPORT_DIMS,
                                     context->max_viewport_dims);

  glitz_initiate_state (&_glitz_glx_gl_proc_address);
    
  context->initialized = 1;
}

static void
_glitz_glx_context_make_current (glitz_glx_drawable_t *drawable,
                                 glitz_bool_t         flush)
{
  if (flush)
    glFlush ();

  glXMakeCurrent (drawable->screen_info->display_info->display,
                  drawable->drawable,
                  drawable->context->context);

  drawable->base.update_all = 1;
  
  if (!drawable->context->initialized)
    _glitz_glx_context_initialize (drawable->screen_info, drawable->context);
}

static void
_glitz_glx_context_update (glitz_glx_drawable_t *drawable,
                           glitz_constraint_t   constraint)
{
  GLXContext context;
  
  switch (constraint) {
  case GLITZ_NONE:
    break;
  case GLITZ_ANY_CONTEXT_CURRENT:
    context = glXGetCurrentContext ();
    if (context == (GLXContext) 0)
      _glitz_glx_context_make_current (drawable, 0);
    break;
  case GLITZ_CONTEXT_CURRENT:
    context = glXGetCurrentContext ();
    if (context != drawable->context->context)
      _glitz_glx_context_make_current (drawable, (context)? 1: 0);
    break;
  case GLITZ_DRAWABLE_CURRENT:
    context = glXGetCurrentContext ();
    if ((context != drawable->context->context) ||
        (glXGetCurrentDrawable () != drawable->drawable))
      _glitz_glx_context_make_current (drawable, (context)? 1: 0);
    break;
  }
}

void
glitz_glx_push_current (void               *abstract_drawable,
                        glitz_surface_t    *surface,
                        glitz_constraint_t constraint)
{
  glitz_glx_drawable_t *drawable = (glitz_glx_drawable_t *) abstract_drawable;
  glitz_glx_context_info_t *context_info;
  int index;

  index = drawable->screen_info->context_stack_size++;

  context_info = &drawable->screen_info->context_stack[index];
  context_info->drawable = drawable;
  context_info->surface = surface;
  context_info->constraint = constraint;
  
  _glitz_glx_context_update (context_info->drawable, constraint);
}

glitz_surface_t *
glitz_glx_pop_current (void *abstract_drawable)
{
  glitz_glx_drawable_t *drawable = (glitz_glx_drawable_t *) abstract_drawable;
  glitz_glx_context_info_t *context_info = NULL;
  int index;

  drawable->screen_info->context_stack_size--;
  index = drawable->screen_info->context_stack_size - 1;

  context_info = &drawable->screen_info->context_stack[index];

  if (context_info->drawable)
    _glitz_glx_context_update (context_info->drawable,
                               context_info->constraint);
  
  if (context_info->constraint == GLITZ_DRAWABLE_CURRENT)
      return context_info->surface;
  
  return NULL;
}