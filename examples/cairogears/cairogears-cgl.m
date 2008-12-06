/*
 * Copyright © 2004 David Reveman, Peter Nilsson
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

#include "cairogears.h"
#include <glitz-cgl.h>
#include <OpenGL/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <strings.h>

#import <Cocoa/Cocoa.h>

static cairo_surface_t *surface = NULL;
static int test_type = 0;
static int aa = -1;
static unsigned int width, height;


static NSAutoreleasePool *pool;
static NSObject *glitzMain;

@interface GlitzOpenGLView : NSView
- (id)initWithFrame:(NSRect)frameRect;
- (BOOL)isOpaque;
- (void)update;
- (void) _surfaceNeedsUpdate:(NSNotification*)notification;
@end

@interface GlitzMain : NSObject

@end

@interface GlitzNSApplication : NSApplication
- (NSEvent *)nextEventMatchingMask:(unsigned int)mask untilDate:(NSDate *)expiration inMode:(NSString *)mode dequeue:(BOOL)flag;
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



@implementation GlitzMain

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
    return YES;
}

@end

/* The implementation class; we reimplement nextEventMatchingMask to draw
   the next frame when idle.  */
@implementation GlitzNSApplication : NSApplication

- (NSEvent *)nextEventMatchingMask:(unsigned int)mask untilDate:(NSDate *)expiration inMode:(NSString *)mode dequeue:(BOOL)flag
{
    NSDate *myExpiration = drawable ? [NSDate distantPast] : expiration;
    do {
        NSEvent *event = [super nextEventMatchingMask:mask
				untilDate:myExpiration
				inMode:mode dequeue:flag];
	
	if(event || !drawable)
	    return event;

	render (test_type, YES, width, height);
    }
    while (expiration
	   && [expiration compare: [NSDate date]] == NSOrderedDescending);
    
    return nil;
}
@end

static NSView *
create_window (int width, int height)
{
    NSRect contentRect;
    NSWindow *window;
    NSView *view;
    
    contentRect = NSMakeRect (50, 50, width, height);

    /* Manually create a window, avoids having a nib file resource.  */
    window = [ [ NSWindow alloc ] 
            initWithContentRect: contentRect
            styleMask: NSTitledWindowMask  | NSMiniaturizableWindowMask
	               | NSClosableWindowMask
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
    [ window makeMainWindow ];
    return view;
}

int
main (int argc, char **argv)
{
    ProcessSerialNumber psn;
    NSView *view;

    glitz_drawable_format_t templ;
    glitz_drawable_format_t *dformat;
    unsigned long mask = 0;
    
    int i;

    program_name = argv[0];
    
    for (i = 1; i < argc; i++) {
	if (!strcasecmp ("-cgl", argv[i])) {
	    ;
	} else if (!strcasecmp ("-noaa", argv[i])) {
	    aa = 0;
	} else if (!strcasecmp ("-swaa", argv[i])) {
	    aa = 1;
	} else if (!strcasecmp ("-hwaa", argv[i])) {
	    aa = 3;
	} else {
	    test_type = get_test_type (argv[i]);
	}
    }
  
    if (!test_type) {
	usage();
	exit(1);
    }

    width = WINDOW_WIDTH;
    height = WINDOW_HEIGHT;
    
    if (aa == 3)
	templ.samples = 4;
    else
	templ.samples = 1;
    
    mask |= GLITZ_FORMAT_SAMPLES_MASK;
    
    if (!GetCurrentProcess(&psn)) {
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);
        SetFrontProcess(&psn);
    }

    pool = [[NSAutoreleasePool alloc] init];
    [GlitzNSApplication sharedApplication];
    
    view = create_window (width, height);
    
    dformat = glitz_cgl_find_window_format (mask, &templ, 0);
    if (!dformat) {
        fprintf (stderr, "Error: couldn't find window format\n");
        exit (1);
    }
  
    drawable = glitz_cgl_create_drawable_for_view (dformat, view,
						   width, height);
    if (!drawable) {
	printf ("failed to create glitz drawable\n");
	exit (1);
    }

    if (aa == 3 && dformat->samples < 2) {
	fprintf (stderr, "hardware multi-sampling not available\n");
	exit (1);
    }

    if (drawable) {
	surface = resize_glitz_drawable (drawable, dformat, width, height);
    }

    cr = cairo_create (surface);
    cairo_set_tolerance (cr, 0.5);

    setup (test_type, width, height);

    /* Create GlitzMain and make it the app delegate */
    glitzMain = [[GlitzMain alloc] init];
    [NSApp setDelegate: glitzMain];
    [NSApp run];
    exit (0);
}
