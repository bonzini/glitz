/*
 * Copyright © 2004 David Reveman
 * Copyright © 2008 Paolo Bonzini
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of the
 * copyright holders not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * The copyright holders make no representations about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID REVEMAN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 * Author: Paolo Bonzini <bonzini@gnu.org>
 */

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitz_cglint.h"

extern glitz_gl_proc_address_list_t _glitz_cgl_gl_proc_address;

static CFBundleRef
_glitz_cgl_get_bundle (const char *name)
{
    CFBundleRef bundle = 0;
    FSRefParam ref_param;
    char framework_name[256];

    framework_name[0] = strlen (name);
    strcpy (&framework_name[1], name);

    memset (&ref_param, 0, sizeof (ref_param));

    if (FindFolder (kSystemDomain,
		    kFrameworksFolderType,
		    kDontCreateFolder,
		    &ref_param.ioVRefNum,
		    &ref_param.ioDirID) == noErr) {
	FSRef ref;

	memset (&ref, 0, sizeof (ref));

	ref_param.ioNamePtr = (unsigned char *) framework_name;
	ref_param.newRef = &ref;

	if (PBMakeFSRefSync (&ref_param) == noErr) {
	    CFURLRef url;

	    url = CFURLCreateFromFSRef (kCFAllocatorDefault, &ref);
	    if (url) {
		bundle = CFBundleCreate (kCFAllocatorDefault, url);
		CFRelease (url);

		if (!CFBundleLoadExecutable (bundle)) {
		    CFRelease (bundle);
		    return (CFBundleRef) 0;
		}
	    }
	}
    }

    return bundle;
}

static void
_glitz_cgl_release_bundle (CFBundleRef bundle)
{
    if (bundle) {
	CFBundleUnloadExecutable (bundle);
	CFRelease (bundle);
    }
}

static void
_glitz_cgl_notify_dummy (void            *abstract_drawable,
			 glitz_surface_t *surface) {}

static glitz_function_pointer_t
_glitz_cgl_get_proc_address (const char *name, void *closure)
{
    glitz_function_pointer_t address = NULL;
    CFBundleRef bundle = (CFBundleRef) closure;
    CFStringRef str;

    if (bundle) {
	str = CFStringCreateWithCString (kCFAllocatorDefault, name,
					 kCFStringEncodingMacRoman);

	address = CFBundleGetFunctionPointerForName (bundle, str);

	CFRelease (str);
    }

    return address;
}

static glitz_context_t *
_glitz_cgl_create_context (void                    *abstract_drawable,
			   glitz_drawable_format_t *format)
{
    NSAutoreleasePool *pool;
    glitz_cgl_drawable_t *drawable = (glitz_cgl_drawable_t *)
	abstract_drawable;
    glitz_cgl_thread_info_t *thread_info = drawable->thread_info;
    glitz_cgl_context_t *context;

    context = malloc (sizeof (glitz_cgl_context_t));
    if (!context)
	return NULL;

    pool = [[NSAutoreleasePool alloc] init];
    context->context =
	[[NSOpenGLContext alloc]
	 initWithFormat: (thread_info->pixel_formats[format->id])
	 shareContext: thread_info->root_context];

    _glitz_context_init (&context->base, &drawable->base);

    context->pbuffer = 0;

    [pool release];
    return (glitz_context_t *) context;
}

static void
_glitz_cgl_context_destroy (void *abstract_context)
{
    glitz_cgl_context_t *context = (glitz_cgl_context_t *) abstract_context;
    glitz_cgl_drawable_t *drawable = (glitz_cgl_drawable_t *)
	context->base.drawable;

    if (drawable->thread_info->cctx == &context->base)
    {
	[NSOpenGLContext clearCurrentContext];
	drawable->thread_info->cctx = NULL;
    }

    [context->context release];

    _glitz_context_fini (&context->base);

    free (context);
}

static void
_glitz_cgl_copy_context (void          *abstract_src,
			 void          *abstract_dst,
			 unsigned long mask)
{
    glitz_cgl_context_t  *src = (glitz_cgl_context_t *) abstract_src;
    glitz_cgl_context_t  *dst = (glitz_cgl_context_t *) abstract_dst;

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [dst->context copyAttributesFromContext: src->context withMask: mask];
    [pool release];
}

static void
_glitz_cgl_make_current (void *abstract_drawable,
			 void *abstract_context)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    glitz_cgl_context_t  *context = (glitz_cgl_context_t *) abstract_context;
    glitz_cgl_drawable_t *drawable = (glitz_cgl_drawable_t *)
	abstract_drawable;
    int update = 0;

    if (drawable->base.width  != drawable->width ||
	drawable->base.height != drawable->height)
	glitz_cgl_drawable_update_size (drawable,
					drawable->base.width,
					drawable->base.height);

    if ([NSOpenGLContext currentContext] != context->context)
	update = 1;
    else if (context->pbuffer != !!drawable->pbuffer)
	update = 1;
    else {
	if (drawable->pbuffer)
	    update |= ([context->context pixelBuffer] != drawable->pbuffer);
	else if (drawable->view)
	    update |= ([context->context view] != drawable->view);
    }

    if (update)
    {
	if (drawable->thread_info->cctx)
	{
	    glitz_context_t *ctx = drawable->thread_info->cctx;

	    if (ctx->lose_current)
		ctx->lose_current (ctx->closure);
	}

	if (drawable->pbuffer) {
	    [context->context setPixelBuffer: drawable->pbuffer
		cubeMapFace: 0 mipMapLevel: 0
		currentVirtualScreen: [context->context currentVirtualScreen]];
	    context->pbuffer = 1;
	}
	else
	{
	    if (context->pbuffer) {
		[context->context clearDrawable];
		context->pbuffer = 0;
	    }
	    [context->context setView: drawable->view];
	}

	[context->context makeCurrentContext];
    }

    drawable->thread_info->cctx = &context->base;
    [pool release];
}

static glitz_function_pointer_t
_glitz_cgl_context_get_proc_address (void       *abstract_context,
				     const char *name)
{
    glitz_cgl_context_t  *context = (glitz_cgl_context_t *) abstract_context;
    glitz_cgl_drawable_t *drawable = (glitz_cgl_drawable_t *)
	context->base.drawable;
    glitz_function_pointer_t func;
    CFBundleRef bundle;

    _glitz_cgl_make_current (drawable, context);

    bundle = _glitz_cgl_get_bundle ("OpenGL.framework");

    func = _glitz_cgl_get_proc_address (name, (void *) bundle);

    _glitz_cgl_release_bundle (bundle);

    return func;
}

glitz_cgl_context_t *
glitz_cgl_context_get (glitz_cgl_thread_info_t *thread_info,
		       glitz_drawable_format_t *format)
{
    glitz_cgl_context_t *context;
    glitz_cgl_context_t **contexts = thread_info->contexts;
    int index, n_contexts = thread_info->n_contexts;

    for (; n_contexts; n_contexts--, contexts++)
	if ((*contexts)->id == format->id)
	    return *contexts;

    index = thread_info->n_contexts++;

    thread_info->contexts =
	realloc (thread_info->contexts,
		 sizeof (glitz_cgl_context_t *) * thread_info->n_contexts);
    if (!thread_info->contexts)
	return NULL;

    context = malloc (sizeof (glitz_cgl_context_t));
    if (!context)
	return NULL;

    thread_info->contexts[index] = context;

    context->context =
	[[NSOpenGLContext alloc]
	 initWithFormat: thread_info->pixel_formats[format->id]
	 shareContext: thread_info->root_context];
    if (!context->context) {
	free (context);
	return NULL;
    }

    context->id = format->id;
    context->pbuffer = 0;

    if (!thread_info->root_context)
	thread_info->root_context = context->context;

    context->backend.gl = &_glitz_cgl_gl_proc_address;

    context->backend.create_pbuffer = glitz_cgl_create_pbuffer;
    context->backend.destroy = glitz_cgl_destroy;
    context->backend.push_current = glitz_cgl_push_current;
    context->backend.pop_current = glitz_cgl_pop_current;
    context->backend.attach_notify = _glitz_cgl_notify_dummy;
    context->backend.detach_notify = _glitz_cgl_notify_dummy;
    context->backend.swap_buffers = glitz_cgl_swap_buffers;
    context->backend.copy_sub_buffer = glitz_cgl_copy_sub_buffer;

    context->backend.create_context = _glitz_cgl_create_context;
    context->backend.destroy_context = _glitz_cgl_context_destroy;
    context->backend.copy_context = _glitz_cgl_copy_context;
    context->backend.make_current = _glitz_cgl_make_current;
    context->backend.get_proc_address = _glitz_cgl_context_get_proc_address;

    context->backend.draw_buffer = _glitz_drawable_draw_buffer;
    context->backend.read_buffer = _glitz_drawable_read_buffer;

    context->backend.drawable_formats = NULL;
    context->backend.n_drawable_formats = 0;

    if (thread_info->n_formats)
    {
	int size;

	size = sizeof (glitz_int_drawable_format_t) * thread_info->n_formats;
	context->backend.drawable_formats = malloc (size);
	if (context->backend.drawable_formats)
	{
	    memcpy (context->backend.drawable_formats, thread_info->formats,
		    size);
	    context->backend.n_drawable_formats = thread_info->n_formats;
	}
    }

    context->backend.texture_formats = NULL;
    context->backend.formats = NULL;
    context->backend.n_formats = 0;

    context->backend.program_map = &thread_info->program_map;
    context->backend.feature_mask = 0;

    context->initialized = 0;

    return context;
}

void
glitz_cgl_context_destroy (glitz_cgl_thread_info_t *thread_info,
			   glitz_cgl_context_t *context)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    if (context->backend.drawable_formats)
	free (context->backend.drawable_formats);

    if (context->backend.formats)
	free (context->backend.formats);

    if (context->backend.texture_formats)
	free (context->backend.texture_formats);

    [context->context release];
    [pool release];
    free (context);
}

static void
_glitz_cgl_context_initialize (glitz_cgl_thread_info_t *thread_info,
			       glitz_cgl_context_t     *context)
{
    CFBundleRef bundle;

    bundle = _glitz_cgl_get_bundle ("OpenGL.framework");

    glitz_backend_init (&context->backend,
			_glitz_cgl_get_proc_address,
			(void *) bundle);

    _glitz_cgl_release_bundle (bundle);

    glitz_initiate_state (&_glitz_cgl_gl_proc_address);

    context->initialized = 1;
}

static void
_glitz_cgl_context_make_current (glitz_cgl_drawable_t *drawable,
				 glitz_bool_t         finish)
{
    NSAutoreleasePool *pool;
    if (finish)
	glFinish ();

    if (drawable->thread_info->cctx)
    {
	glitz_context_t *ctx = drawable->thread_info->cctx;

	if (ctx->lose_current)
	    ctx->lose_current (ctx->closure);

	drawable->thread_info->cctx = NULL;
    }

    pool = [[NSAutoreleasePool alloc] init];
    if (drawable->pbuffer) {
	[drawable->context->context setPixelBuffer: drawable->pbuffer
	    cubeMapFace: 0 mipMapLevel: 0
	    currentVirtualScreen: [drawable->context->context currentVirtualScreen]];
	drawable->context->pbuffer = 1;
    } else {
	if (drawable->context->pbuffer) {
	    [drawable->context->context clearDrawable];
	    drawable->context->pbuffer = 0;
	}

	[drawable->context->context setView: drawable->view];
    }

    [drawable->context->context makeCurrentContext];

    drawable->base.update_all = 1;

    if (!drawable->context->initialized)
	_glitz_cgl_context_initialize (drawable->thread_info,
				       drawable->context);
    [pool release];
}

static void
_glitz_cgl_context_update (glitz_cgl_drawable_t *drawable,
			   glitz_constraint_t   constraint)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSOpenGLContext *context;

    drawable->base.flushed = drawable->base.finished = 0;

    switch (constraint) {
    case GLITZ_NONE:
	break;
    case GLITZ_ANY_CONTEXT_CURRENT:
	context = [NSOpenGLContext currentContext];
	if (context == NULL)
	    _glitz_cgl_context_make_current (drawable, 0);
	break;
    case GLITZ_CONTEXT_CURRENT:
	context = [NSOpenGLContext currentContext];
	if (context != drawable->context->context)
	    _glitz_cgl_context_make_current (drawable, (context)? 1: 0);
	break;
    case GLITZ_DRAWABLE_CURRENT:
	if (drawable->base.width  != drawable->width ||
	    drawable->base.height != drawable->height)
	    glitz_cgl_drawable_update_size (drawable,
					    drawable->base.width,
					    drawable->base.height);

	context = [NSOpenGLContext currentContext];
	if (context != drawable->context->context) {
	    _glitz_cgl_context_make_current (drawable, (context)? 1: 0);
	} else {
	    if (drawable->pbuffer) {
		if ([drawable->context->context pixelBuffer]
		    != drawable->pbuffer)
		    _glitz_cgl_context_make_current (drawable,
						     (context)? 1: 0);

	    } else if (drawable->view) {
		if ([drawable->context->context view] != drawable->view)
		    _glitz_cgl_context_make_current (drawable,
						     (context)? 1: 0);
	    }
	}
	break;
    }
    [pool release];
}

glitz_bool_t
glitz_cgl_push_current (void               *abstract_drawable,
			glitz_surface_t    *surface,
			glitz_constraint_t constraint,
			glitz_bool_t       *restore_state)
{
    glitz_cgl_drawable_t *drawable = (glitz_cgl_drawable_t *)
	abstract_drawable;
    glitz_cgl_context_info_t *context_info;
    int index;

    if (restore_state)
	*restore_state = 0;

    index = drawable->thread_info->context_stack_size++;

    context_info = &drawable->thread_info->context_stack[index];
    context_info->drawable = drawable;
    context_info->surface = surface;
    context_info->constraint = constraint;

    _glitz_cgl_context_update (context_info->drawable, constraint);

    return 1;
}

glitz_surface_t *
glitz_cgl_pop_current (void *abstract_drawable)
{
    glitz_cgl_drawable_t *drawable = (glitz_cgl_drawable_t *)
	abstract_drawable;
    glitz_cgl_context_info_t *context_info = NULL;
    int index;

    drawable->thread_info->context_stack_size--;
    index = drawable->thread_info->context_stack_size - 1;

    context_info = &drawable->thread_info->context_stack[index];

    if (context_info->drawable)
	_glitz_cgl_context_update (context_info->drawable,
				   context_info->constraint);

    if (context_info->constraint == GLITZ_DRAWABLE_CURRENT)
	return context_info->surface;

    return NULL;
}
