#ifndef _CAIROGEARS_H
#define _CAIROGEARS_H 1

#include <cairo.h>
#include <glitz.h>
#include <cairo-glitz.h>

#define PACKAGE "cairogears"

#define WINDOW_WIDTH  512
#define WINDOW_HEIGHT 512

extern cairo_t *cr;
extern unsigned int frame_cnt;
extern char *program_name;

void trap_setup (cairo_t *cr, int w, int h);
void trap_render (int w, int h, int fill_gradient);

void comp_setup (cairo_t *cr, int w, int h);
void comp_render (int w, int h);

void text_setup (cairo_t *cr, int w, int h);
void text_render (int w, int h);

void text2_setup (cairo_t *cr, int w, int h);
void text2_render (int w, int h);

void shadow_setup (cairo_t *cr, int w, int h);
void shadow_render (int w, int h);

enum {
    IMAGE_TYPE = 1,
    XRENDER_TYPE,
    GLX_TYPE,
    AGL_TYPE
};

enum {
    STROKE_AND_FILL_TYPE = 1,
    STROKE_AND_FILL_TYPE_GRADIENT,
    COMPOSITE_AND_TRANSFORM_TYPE,
    TEXT_PATH_TYPE,
    SHADOW_TYPE
};

void usage(void);
void resize_image (int w, int h);
void alarmhandler (int sig);
int get_test_type (const char *arg);
glitz_format_t *get_glitz_format (glitz_drawable_format_t* dformat,
		                  glitz_drawable_t* drawable);
cairo_surface_t * resize_glitz_drawable (glitz_drawable_t *drawable,
				         glitz_drawable_format_t *dformat,
					 int width, int height);
cairo_surface_t *cairo_glitz_surface_create_from_png (cairo_t *glitz_target,
						      char *name, int *width,
						      int *height);

#endif
