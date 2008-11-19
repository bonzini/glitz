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
#  include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>

#include "rendertest.h"
#include "glitz_common.h"
#include <glitz-cgl.h>
#import <Cocoa/Cocoa.h>


static NSAutoreleasePool *pool;
static NSObject *glitzMain;
static int rendertest_main (void);

@interface GlitzOpenGLView : NSView
- (id)initWithFrame:(NSRect)frameRect;
- (BOOL)isOpaque;
- (void)update;
- (void) _surfaceNeedsUpdate:(NSNotification*)notification;
@end

@interface GlitzMain : NSObject
@end

/* The view class.  A (very...) simplified version of CustomOpenGLView
   from the Custom Cocoa OpenGL example.  */

@implementation GlitzOpenGLView

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    [[NSNotificationCenter defaultCenter]
	addObserver:self selector:@selector(_surfaceNeedsUpdate:)
	name:NSViewGlobalFrameDidChangeNotification object:self];
    return self;
}

- (BOOL)isOpaque
{
    return YES;
}

- (void)update
{
    if ([[NSOpenGLContext currentContext] view] == self) {
        [[NSOpenGLContext currentContext] update];
    }
}

- (void) _surfaceNeedsUpdate:(NSNotification*)notification
{
    [self update];
}

@end



/* The application delegate, which has the only purpose of firing
   rendertest's main.  */

@implementation GlitzMain

/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
    int status = rendertest_main ();

    [glitzMain release];
    [pool release];
    exit (status);
}

@end

static NSView *
create_window (int width, int height)
{
    NSRect contentRect;
    NSWindow *window;
    NSView *view;
    
    contentRect = NSMakeRect (50, 50, width, height);

    /* Manually create a window, avoids having a nib file resource.  We
       do not care about NSMiniaturizableWindowMask and NSClosableWindowMask
       because we do not handle events.  NSResizableWindowMask wouldn't have
       any effect for the same reason, but we explicitly do not want that.  */
    window = [ [ NSWindow alloc ] 
            initWithContentRect: contentRect
            styleMask: NSTitledWindowMask 
            backing: NSBackingStoreBuffered
            defer: NO ];
                          
    if (window == nil) {
        printf ("Could not create the Cocoa window\n");
	exit (1);
    }

    [ window setTitle: @PACKAGE ];
    [ window setViewsNeedDisplay:NO ];

    view = [ [ [ GlitzOpenGLView alloc ] init ] autorelease ];
    [ window setContentView: view ];
    [ window makeKeyAndOrderFront:nil ];
    return view;
}


static const render_backend_t _glitz_cgl_render_backend = {
  _glitz_render_create_similar,
  _glitz_render_destroy,
  _glitz_render_composite,
  _glitz_render_set_pixels,
  _glitz_render_show,
  _glitz_render_set_fill,
  _glitz_render_set_component_alpha,
  _glitz_render_set_transform,
  _glitz_render_set_filter,
  _glitz_render_set_clip_rectangles,
  _glitz_render_set_clip_trapezoids
};

typedef struct cgl_options {
  int samples;
  int db;
  int offscreen;
} cgl_options_t;

static const render_option_t _cgl_options[] = {
  { "single-buffer", 'l', NULL, 0, "    use single buffered format" },
  { "samples", 'p', "SAMPLES", 0, "   use this hardware multi-sample format" },
  { "offscreen", 'f', NULL, 0, "        use offscreen rendering" },
  { 0 }
};

static int
_parse_option (int key, char *arg, render_arg_state_t *state)
{
  cgl_options_t *options = state->pointer;

  switch (key) {
  case 'l':
    options->db = 0;
    break;
  case 'p':
    options->samples = atoi (arg);
    break;
  case 'f':
    options->offscreen = 1;
    break;
  default:
    return 1;
  }

  return 0;
}

static render_arg_state_t state;
static cgl_options_t options  = { 1, 1, 0 };

static int
rendertest_main (void)
{
  int status;
  render_surface_t surface;
  glitz_drawable_format_t templ;
  unsigned long mask;
  glitz_drawable_format_t *dformat;
  glitz_drawable_t *drawable, *offscreen = 0;
  NSView *view;

  surface.backend = &_glitz_cgl_render_backend;
  surface.flags = 0;
  surface.width = RENDER_DEFAULT_DST_WIDTH;
  surface.height = RENDER_DEFAULT_DST_HEIGHT;

  view = create_window (RENDER_DEFAULT_DST_WIDTH, RENDER_DEFAULT_DST_HEIGHT);

  templ.samples = options.samples;
  mask = GLITZ_FORMAT_SAMPLES_MASK;

  if (options.db)
    templ.doublebuffer = 1;
  else
    templ.doublebuffer = 0;

  mask |= GLITZ_FORMAT_DOUBLEBUFFER_MASK;

  dformat = glitz_cgl_find_window_format (mask, &templ, 0);
  if (!dformat) {
    fprintf (stderr, "Error: couldn't find window format\n");
    return 1;
  }

  drawable =
    glitz_cgl_create_drawable_for_view (dformat, view,
					surface.width, surface.height);
  if (!drawable) {
    fprintf (stderr, "Error: couldn't create glitz drawable for view\n");
    return 1;
  }

  if (options.offscreen)
  {
      dformat = glitz_find_drawable_format (drawable, 0, 0, 0);
      if (!dformat)
      {
	  fprintf (stderr, "Error: couldn't find offscreen format\n");
	  return 1;
      }

      offscreen = glitz_create_drawable (drawable, dformat,
					 surface.width, surface.height);
      if (!offscreen)
      {
	  fprintf (stderr, "Error: couldn't create offscreen drawable\n");
	  return 1;
      }

      surface.surface =
	  _glitz_create_and_attach_surface_to_drawable (drawable,
							offscreen,
							surface.width,
							surface.height);
  }
  else
  {
    surface.surface =
      _glitz_create_and_attach_surface_to_drawable (drawable,
						    drawable,
						    surface.width,
						    surface.height);
  }

  if (!surface.surface)
    return 1;

  status = render_run (&surface, &state.settings);

  glitz_surface_destroy ((glitz_surface_t *) surface.surface);

  if (offscreen)
      glitz_drawable_destroy (offscreen);

  glitz_drawable_destroy (drawable);

  glitz_cgl_fini ();

  return status;
}


int main(int argc, char *argv[])
{
    ProcessSerialNumber psn;

    state.pointer = &options;
    if (render_parse_arguments (_parse_option,
			        _cgl_options,
			        &state,
			        argc, argv))
	return 1;

    /* Add the icon to the dock.  */
    if (!GetCurrentProcess(&psn)) {
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);
        SetFrontProcess(&psn);
    }

    pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    [NSApp finishLaunching];
    
    /* Create GlitzMain and make it the app delegate */
    glitzMain = [[GlitzMain alloc] init];
    [NSApp setDelegate: glitzMain];
    [NSApp run];
    return 0;
}
