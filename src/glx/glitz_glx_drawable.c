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

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitz_glxint.h"

static glitz_glx_drawable_t *
_glitz_glx_create_drawable (glitz_glx_screen_info_t *screen_info,
			    glitz_glx_context_t     *context,
			    glitz_drawable_format_t *format,
			    GLXDrawable             glx_drawable,
			    GLXPbuffer              glx_pbuffer,
			    GLXPixmap               glx_pixmap,
			    glitz_bool_t            owned,
			    int                     width,
			    int                     height)
{
    glitz_glx_drawable_t *drawable;

    drawable = (glitz_glx_drawable_t *) malloc (sizeof (glitz_glx_drawable_t));
    if (drawable == NULL)
	return NULL;

    drawable->screen_info = screen_info;
    drawable->context = context;
    drawable->drawable = glx_drawable;
    drawable->pbuffer = glx_pbuffer;
    drawable->pixmap = glx_pixmap;
    drawable->owned = owned;
    drawable->width = width;
    drawable->height = height;

    _glitz_drawable_init (&drawable->base,
			  &screen_info->formats[format->id],
			  &context->backend,
			  width, height);

    if (!context->initialized) {
	glitz_glx_push_current (drawable, NULL, GLITZ_CONTEXT_CURRENT, NULL);
	glitz_glx_pop_current (drawable);
    }

    if (width > context->backend.max_viewport_dims[0] ||
	height > context->backend.max_viewport_dims[1]) {
	free (drawable);
	return NULL;
    }

    screen_info->drawables++;

    return drawable;
}

glitz_bool_t
_glitz_glx_drawable_update_size (glitz_glx_drawable_t *drawable,
				 int                  width,
				 int                  height)
{
    if (drawable->pbuffer)
    {
	if (glXGetCurrentDrawable () == drawable->drawable)
	    glXMakeCurrent (drawable->screen_info->display_info->display,
			    None, NULL);

	glitz_glx_pbuffer_destroy (drawable->screen_info, drawable->pbuffer);
	drawable->drawable = drawable->pbuffer =
	    glitz_glx_pbuffer_create (drawable->screen_info,
				      drawable->context->fbconfig,
				      (int) width, (int) height);
	if (!drawable->pbuffer)
	    return 0;
    }
    
    if (drawable->pixmap)
    {
	if (glXGetCurrentDrawable () == drawable->drawable)
	    glXMakeCurrent (drawable->screen_info->display_info->display,
			    None, NULL);
	
	glitz_glx_release_tex_image(drawable);
	glitz_glx_pixmap_destroy(drawable->screen_info, drawable->drawable);
	
	if (drawable->owned)
	{
	    Display* display = drawable->screen_info->display_info->display;
	    int screen = drawable->screen_info->screen;
	    XVisualInfo* vinfo;
	    
	    XFreePixmap(drawable->screen_info->display_info->display, 
			drawable->pixmap);
	    vinfo = glitz_glx_get_visual_info_from_format (display, screen, 
							   &drawable->base.format->d);
	    drawable->pixmap = XCreatePixmap (display, RootWindow(display, screen),
					      width, height, vinfo->depth);
	}
	drawable->drawable = glitz_glx_pixmap_create (drawable->screen_info,
						      drawable->pixmap,
						      &drawable->base.format->d,
						      width, height);
    }
    
    drawable->width  = width;
    drawable->height = height;
    
    return 1;
}

static glitz_drawable_t *
_glitz_glx_create_pixmap_drawable (glitz_glx_screen_info_t *screen_info,
				   glitz_drawable_format_t *format,
				   Pixmap                  pixmap,
				   glitz_bool_t            owned,
				   unsigned int            width,
				   unsigned int            height)
{
    glitz_glx_drawable_t        *drawable;
    glitz_glx_context_t         *context;
    GLXPixmap glx_pixmap;
    
    context = glitz_glx_context_get (screen_info, format);
    if (!context)
	return NULL;

    glx_pixmap = glitz_glx_pixmap_create(screen_info, pixmap, format, width, height);
    if (!glx_pixmap)
	return NULL;
    
    drawable = _glitz_glx_create_drawable (screen_info, context, format,
					   glx_pixmap, (GLXPbuffer) 0, 
					   pixmap, owned, 
					   width, height);
    
    if (!drawable)
    {
	glitz_glx_pixmap_destroy (screen_info, glx_pixmap);
	return NULL;
    }
    
    return &drawable->base;
}

static glitz_drawable_t *
_glitz_glx_create_pbuffer_drawable (glitz_glx_screen_info_t *screen_info,
				    glitz_drawable_format_t *format,
				    unsigned int            width,
				    unsigned int            height)
{
    glitz_glx_drawable_t *drawable;
    glitz_glx_context_t *context;
    GLXPbuffer pbuffer;

    context = glitz_glx_context_get (screen_info, format);
    if (!context)
	return NULL;

    pbuffer = glitz_glx_pbuffer_create (screen_info, context->fbconfig,
					(int) width, (int) height);
    if (!pbuffer)
	return NULL;

    drawable = _glitz_glx_create_drawable (screen_info, context, format,
					   pbuffer, pbuffer, (GLXPixmap) 0, 
					   None, width, height);
    if (!drawable) {
	glitz_glx_pbuffer_destroy (screen_info, pbuffer);
	return NULL;
    }

    return &drawable->base;
}

glitz_drawable_t *
glitz_glx_create_pixmap (void                    *abstract_templ,
			 glitz_drawable_format_t *format,
			 unsigned int            width,
			 unsigned int            height)
{
    glitz_glx_screen_info_t *templ = (glitz_glx_screen_info_t *) abstract_templ;
    glitz_drawable_t     *drawable;
    Display              *display = templ->display_info->display;
    int                  screen = templ->screen;
    Pixmap               pixmap;
    
    pixmap = XCreatePixmap (display, RootWindow(display, screen),
			    width, height, format->depth);
    if (!pixmap)
	return NULL;
    
    drawable = _glitz_glx_create_pixmap_drawable (templ, format, 
						  pixmap, 1, width, height);
    if (!drawable)
    {
	XFreePixmap (display, pixmap);
	return NULL;
    }
    
    return drawable;
}

glitz_drawable_t *
glitz_glx_create_pbuffer (void                    *abstract_templ,
			  glitz_drawable_format_t *format,
			  unsigned int            width,
			  unsigned int            height)
{
    glitz_glx_drawable_t *templ = (glitz_glx_drawable_t *) abstract_templ;

    return _glitz_glx_create_pbuffer_drawable (templ->screen_info, format,
					       width, height);
}

/**
 * glitz_glx_create_drawable_for_window:
 * @display: an X Display
 * @screen: X Screen number associated with @window
 * @format: format to use for drawing to @window. The format must be set by
 *          glitz_glx_find_window_format () or 
 *          glitz_glx_find_drawable_format_for_visual () before call this 
 *          function.
 * @window: a X Xindow or a X Pixmap
 * @width: the current width of @window
 * @height: the current height of @window
 *
 * Create a glitz drawable that draws to the given window.
 *
 * Return value: the newly glitz drawable. The caller owns the drawable and 
 *               should call glitz_drawable_destroy() when done with it. The
 *               function return nil if fail.
 **/
glitz_drawable_t *
glitz_glx_create_drawable_for_window (Display                 *display,
				      int                     screen,
				      glitz_drawable_format_t *format,
				      Window                  window,
				      unsigned int            width,
				      unsigned int            height)
{
    glitz_glx_drawable_t        *drawable;
    glitz_glx_screen_info_t     *screen_info;
    glitz_glx_context_t         *context;
    glitz_int_drawable_format_t *iformat;

    screen_info = glitz_glx_screen_info_get (display, screen);
    if (!screen_info)
	return NULL;

    if (format->id >= screen_info->n_formats)
	return NULL;

    iformat = &screen_info->formats[format->id];
    if (!(iformat->types & GLITZ_DRAWABLE_TYPE_WINDOW_MASK))
	return NULL;

    context = glitz_glx_context_get (screen_info, format);
    if (!context)
	return NULL;

    drawable = _glitz_glx_create_drawable (screen_info, context, format,
					   window, (GLXPbuffer) 0, (GLXPixmap)0,
					   None, width, height);
    if (!drawable)
	return NULL;

    return &drawable->base;
}
slim_hidden_def(glitz_glx_create_drawable_for_window);

/**
 * glitz_glx_create_drawable_for_pixmap:
 * @display: an X Display
 * @screen: X Screen number associated with @pixmap
 * @format: format to use for drawing to @pixmap. The format must be set by
 *          glitz_glx_find_window_format () or 
 *          glitz_glx_find_drawable_format_for_visual () before call this 
 *          function.
 * @pixmap: a X Pixmap
 * @width: the current width of @pixmap
 * @height: the current height of @pixmap
 *
 * Create a glitz drawable that draws to the given pixmap. The main difference 
 * with glitz_glx_create_drawable_for_window () is this function associate a 
 * OpenGL offscreen pixmap with @pixmap which provide a shared drawing between
 * the OpenGL offscreen pixmap and @pixmap.
 *
 * Return value: the newly glitz drawable. The caller owns the drawable and 
 *               should call glitz_drawable_destroy() when done with it. The
 *               function return nil if fail.
 **/
glitz_drawable_t *
glitz_glx_create_drawable_for_pixmap (Display                 *display,
				      int                     screen,
				      glitz_drawable_format_t *format,
				      Pixmap                  pixmap,
				      unsigned int            width,
				      unsigned int            height)
{
    glitz_glx_screen_info_t     *screen_info;
    
    screen_info = glitz_glx_screen_info_get (display, screen);
    if (!screen_info)
	return NULL;
    
    if (format->id >= screen_info->n_formats)
	return NULL;
    
    return _glitz_glx_create_pixmap_drawable (screen_info, format, pixmap, 0,
					      width, height);
}
slim_hidden_def(glitz_glx_create_drawable_for_pixmap);

/**
 * glitz_glx_create_pbuffer_drawable:
 * @display: an X Display
 * @screen: X Screen number
 * @format: format to use for drawing to pixel buffer object. The format must be 
 *          set by glitz_glx_find_pbuffer_format () before call this function.
 * @width: the width of pixel buffer object.
 * @height: the height of pixel buffer object.
 *
 * Create an offscreen pixel buffer object and create a glitz drawable that 
 * draws onto it.
 *
 * Return value: the newly glitz drawable. The caller owns the drawable and 
 *               should call glitz_drawable_destroy() when done with it. The
 *               function return nil if fail.
 **/
glitz_drawable_t *
glitz_glx_create_pbuffer_drawable (Display                 *display,
				   int                     screen,
				   glitz_drawable_format_t *format,
				   unsigned int            width,
				   unsigned int            height)
{
    glitz_glx_screen_info_t     *screen_info;
    glitz_int_drawable_format_t *iformat;

    screen_info = glitz_glx_screen_info_get (display, screen);
    if (!screen_info)
	return NULL;

    if (format->id >= screen_info->n_formats)
	return NULL;

    iformat = &screen_info->formats[format->id];
    if (!(iformat->types & GLITZ_DRAWABLE_TYPE_PBUFFER_MASK))
	return NULL;

    return _glitz_glx_create_pbuffer_drawable (screen_info, format,
					       width, height);
}
slim_hidden_def(glitz_glx_create_pbuffer_drawable);

/**
 * glitz_glx_create_pixmap_drawable:
 * @display: an X Display
 * @screen: X Screen number
 * @format: format to use for drawing to pixmap. The format must be set by
 *          glitz_glx_find_window_format () or 
 *          glitz_glx_find_drawable_format_for_visual () before call this 
 *          function.
 * @width: the width of pixmap.
 * @height: the height of pixmap.
 *
 * Create an offscreen OpenGL pixmap and create a glitz drawable that 
 * draws onto it. The X Pixmap associated to the OpenGL pixmap can be recover 
 * with glitz_glx_get_xdrawable (). This function is similar than
 * glitz_glx_create_drawable_for_pixmap () except that this function create the 
 * X Pixmap associated at OpenGL offscreen pixmap.
 *
 * Return value: the newly glitz drawable. The caller owns the drawable and 
 *               should call glitz_drawable_destroy() when done with it. The
 *               function return nil if fail.
 **/
glitz_drawable_t *
glitz_glx_create_pixmap_drawable (Display                 *display,
				  int                     screen,
				  glitz_drawable_format_t *format,
				  unsigned int            width,
				  unsigned int            height)
{
    glitz_drawable_t            *drawable;
    glitz_glx_screen_info_t     *screen_info;
    
    screen_info = glitz_glx_screen_info_get (display, screen);
    if (!screen_info)
	return NULL;
    
    if (format->id >= screen_info->n_formats)
	return NULL;
    
    drawable = glitz_glx_create_pixmap (screen_info, format, width, height);
    
    return drawable;
}
slim_hidden_def(glitz_glx_create_pixmap_drawable);

/**
 * glitz_glx_get_display_from_drawable:
 * @drawable: a glitz drawable
 *
 * Get the X Display for the underlying X Drawable.
 *
 * Return value: the display. The function return NULL if fail.
 **/
Display*
glitz_glx_get_display_from_drawable(glitz_drawable_t* drawable)
{
    glitz_glx_drawable_t *glx_drawable = (glitz_glx_drawable_t *)drawable;
    
    if (!glx_drawable) 
	return NULL;
    
    return glx_drawable->screen_info->display_info->display;
}
slim_hidden_def(glitz_glx_get_display_from_drawable);

/**
 * glitz_glx_get_screen_from_drawable:
 * @drawable: a glitz drawable
 *
 * Get the X Screen number for the underlying X Drawable.
 *
 * Return value: the screen number. The function return -1 if fail.
 **/
int
glitz_glx_get_screen_from_drawable(glitz_drawable_t* drawable)
{
    glitz_glx_drawable_t *glx_drawable = (glitz_glx_drawable_t *)drawable;
    
    if (!glx_drawable) 
	return -1;
    
    return glx_drawable->screen_info->screen;
}
slim_hidden_def(glitz_glx_get_screen_from_drawable);

/**
 * glitz_glx_get_drawable_from_drawable:
 * @drawable: a glitz drawable
 *
 * Get the underlying X Drawable assiocated to the glitz drawable.
 *
 * Return value: the drawable. The function return None if fail.
 **/
Drawable
glitz_glx_get_drawable_from_drawable(glitz_drawable_t* drawable)
{
    glitz_glx_drawable_t *glx_drawable = (glitz_glx_drawable_t *)drawable;
    Drawable xdrawable = None;
    
    if (glx_drawable->pbuffer)
	return None;
    
    if (glx_drawable->pixmap)
	xdrawable = (Drawable)glx_drawable->pixmap;
    else
	xdrawable = (Drawable)glx_drawable->drawable;
    
    return xdrawable;
}
slim_hidden_def(glitz_glx_get_drawable_from_drawable);

void
glitz_glx_destroy (void *abstract_drawable)
{
    glitz_glx_drawable_t *drawable = (glitz_glx_drawable_t *)
	abstract_drawable;

    drawable->screen_info->drawables--;
    if (drawable->screen_info->drawables == 0) {
	/*
	 * Last drawable? We have to destroy all fragment programs as this may
	 * be our last chance to have a context current.
	 */
	glitz_glx_push_current (abstract_drawable, NULL,
				GLITZ_CONTEXT_CURRENT, NULL);
	glitz_program_map_fini (drawable->base.backend->gl,
				&drawable->screen_info->program_map);
	glitz_program_map_init (&drawable->screen_info->program_map);
	glitz_glx_pop_current (abstract_drawable);
    }

    if (glXGetCurrentDrawable () == drawable->drawable)
	glXMakeCurrent (drawable->screen_info->display_info->display,
			None, NULL);
    
    if (drawable->pixmap)
    {
	glitz_glx_release_tex_image(drawable);
	glitz_glx_pixmap_destroy(drawable->screen_info, drawable->drawable);
	if (drawable->owned)
	    XFreePixmap(drawable->screen_info->display_info->display, 
			drawable->pixmap);
    }
    
    if (drawable->pbuffer)
	glitz_glx_pbuffer_destroy (drawable->screen_info, drawable->pbuffer);

    free (drawable);
}

glitz_bool_t
glitz_glx_swap_buffers (void *abstract_drawable)
{
    glitz_glx_drawable_t *drawable = (glitz_glx_drawable_t *)
	abstract_drawable;

    glXSwapBuffers (drawable->screen_info->display_info->display,
		    drawable->drawable);

    return 1;
}

glitz_bool_t
glitz_glx_copy_sub_buffer (void *abstract_drawable,
			   int  x,
			   int  y,
			   int  width,
			   int  height)
{
    glitz_glx_drawable_t    *drawable = (glitz_glx_drawable_t *)
	abstract_drawable;
    glitz_glx_screen_info_t *screen_info = drawable->screen_info;

    if (screen_info->glx_feature_mask & GLITZ_GLX_FEATURE_COPY_SUB_BUFFER_MASK)
    {
	screen_info->glx.copy_sub_buffer (screen_info->display_info->display,
					  drawable->drawable,
					  x, y, width, height);
	return 1;
    }

    return 0;
}

glitz_bool_t
glitz_glx_bind_tex_image (void *abstract_drawable)
{
    glitz_glx_drawable_t    *drawable = (glitz_glx_drawable_t *)
	abstract_drawable;
    glitz_glx_screen_info_t *screen_info = drawable->screen_info;
    
    if (!(screen_info->glx_feature_mask & 
	  GLITZ_GLX_FEATURE_TEXTURE_FROM_PIXMAP_MASK))
	return 0;
    
    if (!drawable->pixmap)
	return 0;
    
    screen_info->glx.bind_tex_image (screen_info->display_info->display,
				     drawable->drawable, 
				     GLITZ_GL_FRONT_LEFT_EXT, NULL);
    
    return 1;
}

glitz_bool_t
glitz_glx_release_tex_image (void *abstract_drawable)
{
    glitz_glx_drawable_t    *drawable = (glitz_glx_drawable_t *)
	abstract_drawable;
    glitz_glx_screen_info_t *screen_info = drawable->screen_info;
    
    if (!(screen_info->glx_feature_mask & 
	  GLITZ_GLX_FEATURE_TEXTURE_FROM_PIXMAP_MASK))
	return 0;
    
    if (drawable->pixmap)
	screen_info->glx.release_tex_image (screen_info->display_info->display,
					    drawable->drawable, 
					    GLITZ_GL_FRONT_LEFT_EXT);
    
    return 1;
}

void
glitz_glx_query_drawable(void *abstract_drawable, int query, 
			 unsigned int* value)
{
    glitz_glx_drawable_t    *drawable = (glitz_glx_drawable_t *)
	abstract_drawable;
    glitz_glx_screen_info_t *screen_info = drawable->screen_info;
    
    screen_info->glx.query_drawable (screen_info->display_info->display,
				     drawable->drawable, query, value);
}
