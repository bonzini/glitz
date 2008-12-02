#include "cairogears.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <strings.h>

cairo_t *cr;
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
	    "\tSHADOW Composite with mask and convolution filter test\n\n",
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

