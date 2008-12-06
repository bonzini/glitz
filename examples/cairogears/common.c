#include "cairogears.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <strings.h>

cairo_t *cr;
glitz_drawable_t *drawable;
unsigned int frame_cnt = 0;
char *program_name;

void
usage(void) {
  
    printf ("Usage: %s ["
	    "-image | "
	    "-xrender | "
	    "{-glx | -agl} [-noaa|-swaa|-hwaa]"
	    "] TEST\n\n\tThe following tests are available:\n\n"
	    "\tTRAP   Trapezoid fill test\n"
	    "\tGRAD   Trapezoid gradient test\n"
	    "\tCOMP   Composite and transform test\n"
	    "\tTEXT   Text path test\n"
	    "\tSHADOW Composite with mask and convolution filter test\n"
	    "\tOPENGL OpenGL drawing on a Glitz surface\n\n",
	    program_name);
}

void alarmhandler (int sig) {
    if (sig == SIGALRM) {
       {
	printf ("%d frames in 5.0 seconds = %.3f FPS\n", frame_cnt,
		frame_cnt / 5.0);
       }
    frame_cnt = 0; 
    }
    signal (SIGALRM, alarmhandler);
    alarm(5);
}

void
setup (int test_type, int width, int height)
{
    switch (test_type) {
    case STROKE_AND_FILL_TYPE:
	trap_setup (cr, width, height);
	break;
    case COMPOSITE_AND_TRANSFORM_TYPE:
	comp_setup (cr, width, height);
	break;
    case TEXT_PATH_TYPE:
	text_setup (cr, width, height);
	break;
    case SHADOW_TYPE:
	shadow_setup (cr, width, height);
	break;
    case OPENGL_TYPE:
	opengl_setup (cr, width, height);
	break;
    }
  
    signal (SIGALRM, alarmhandler);
    alarm (5);
}

void
render (int test_type, glitz_bool_t swap, int width, int height)
{
    switch (test_type) {
    case STROKE_AND_FILL_TYPE:
    case STROKE_AND_FILL_TYPE_GRADIENT:
	trap_render (width, height, test_type == STROKE_AND_FILL_TYPE_GRADIENT);
	break;
    case COMPOSITE_AND_TRANSFORM_TYPE:
	comp_render (width, height);
	break;
    case TEXT_PATH_TYPE:
	text_render (width, height);
	break;
    case SHADOW_TYPE:
	shadow_render (width, height);
	break;
    case OPENGL_TYPE:
	opengl_render (width, height);
	break;
    }

    if (swap)
        glitz_drawable_swap_buffers (drawable);
    frame_cnt++;
}

glitz_format_t*
get_glitz_format (glitz_drawable_format_t* dformat,
                  glitz_drawable_t* drawable)
{
        glitz_format_t* format = NULL;
        glitz_format_t formatTemplate;

        formatTemplate.color = dformat->color;
        formatTemplate.color.fourcc = GLITZ_FOURCC_RGB;

        format = glitz_find_format (drawable,
                                    GLITZ_FORMAT_RED_SIZE_MASK   |
                                    GLITZ_FORMAT_GREEN_SIZE_MASK |
                                    GLITZ_FORMAT_BLUE_SIZE_MASK  |
                                    GLITZ_FORMAT_ALPHA_SIZE_MASK |
                                    GLITZ_FORMAT_FOURCC_MASK,
                                    &formatTemplate,
                                    0);

        if (!format)
        {
                puts ("Could not find glitz-surface-format!");
                return NULL;
        }

        return format;
}

cairo_surface_t *
resize_glitz_drawable (glitz_drawable_t *drawable,
		       glitz_drawable_format_t *dformat,
		       int width,
		       int height)
{
    glitz_surface_t *surface;
    glitz_format_t *format;
    cairo_surface_t *crsurface;

    glitz_drawable_update_size (drawable, width, height);

    format = get_glitz_format (dformat, drawable);
    surface = glitz_surface_create (drawable, format, width, height, 0, NULL);
    if (!surface) {
	fprintf (stderr, "Error: couldn't create glitz surface\n");
	exit (1);
    }

    if (dformat->doublebuffer)
        glitz_surface_attach (surface, drawable, GLITZ_DRAWABLE_BUFFER_BACK_COLOR);
    else
        glitz_surface_attach (surface, drawable, GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);
    
    crsurface = cairo_glitz_surface_create (surface);
    
    return crsurface;
}


int
get_test_type (const char *arg)
{
    if (!strcasecmp ("TRAP", arg)) {
	return STROKE_AND_FILL_TYPE;
    } else if (!strcasecmp ("GRAD", arg)) {
	return STROKE_AND_FILL_TYPE_GRADIENT;
    } else if (!strcasecmp ("COMP", arg)) {
	return COMPOSITE_AND_TRANSFORM_TYPE;
    } else if (!strcasecmp ("TEXT", arg)) {
	return TEXT_PATH_TYPE;
    } else if (!strcasecmp ("SHADOW", arg)) {
	return SHADOW_TYPE;
    } else if (!strcasecmp ("OPENGL", arg)) {
	return OPENGL_TYPE;
    } else {
	fprintf (stderr, "%s: unrecognized option `%s'\n",
		 program_name, arg);
	usage();
	exit(1);
    }
}

cairo_surface_t *
cairo_glitz_surface_create_from_png (cairo_t *glitz_target,
				     char *name, int *width,
				     int *height)
{
    cairo_surface_t *image_surface;
    cairo_surface_t *glitz_surface;
    cairo_t *cr;

    image_surface = cairo_image_surface_create_from_png (name);
    if (cairo_surface_status (image_surface))
	return image_surface;

    *width = cairo_image_surface_get_width (image_surface);
    *height = cairo_image_surface_get_height (image_surface);

    glitz_surface = cairo_surface_create_similar (cairo_get_target (glitz_target),
						  CAIRO_CONTENT_COLOR_ALPHA,
						  *width, *height);

    cr = cairo_create (glitz_surface);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface (cr, image_surface, 0.0, 0.0);
    cairo_paint (cr);
    cairo_destroy (cr);
    cairo_surface_destroy (image_surface);
    return glitz_surface;
}

