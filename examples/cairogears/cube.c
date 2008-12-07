/*
 * Copyright © 2008 Paolo Bonzini
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the names of
 * Paolo Bonzini not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission. Paolo Bonzini
 * makes no representations about the suitability of this software for
 * any purpose. It is provided "as is" without express or implied warranty.
 *
 * PAOLO BONZINI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL PAOLO BONZINI BE LIABLE FOR ANY SPECIAL, INDIRECT
 * OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Paolo Bonzini <bonzini@gnu.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cairo.h>
#include <math.h>

#include "cairogears.h"
#include "gears.h"
#include "glitz.h"

static glitz_texture_object_t *face_texture;
static glitz_context_t *cube_context;

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#define Z_OFS		10.0
#define CUBE_SIZE	3.0
#define BG_WIDTH	512
#define BG_HEIGHT	512

int list;


static GLfloat cube_vertices [8][3] = {
{1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {-1.0, -1.0, 1.0}, {-1.0, 1.0, 1.0},
{1.0, 1.0, -1.0}, {1.0, -1.0, -1.0}, {-1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0} };

short cube_faces [6][4] = {
{3, 2, 1, 0}, {2, 3, 7, 6}, {0, 1, 5, 4}, {3, 0, 4, 7}, {1, 2, 6, 5}, {4, 5, 6, 7} };

static GLfloat texture_coords [4][2] = {
{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0} };

static void
draw_cube (void)
{
    int i, f;

    glBegin (GL_QUADS);
    for (f = 0; f < 6; f++)
        for (i = 0; i < 4; i++) {
	    double u = texture_coords[i][0] * BG_WIDTH;
	    double v = texture_coords[i][1] * BG_HEIGHT;
	    glitz_texture_object_pixel_to_texture_coord (face_texture, &u, &v);
	    glTexCoord2f (u, v);
	    glVertex3f (cube_vertices[cube_faces[f][i]][0] * CUBE_SIZE / 2,
			cube_vertices[cube_faces[f][i]][1] * CUBE_SIZE / 2,
			cube_vertices[cube_faces[f][i]][2] * CUBE_SIZE / 2);
        }
    glEnd ();
}

void
cube_setup (cairo_t *_cr, int w, int h)
{
    cairo_surface_t *cairo_texture_surface;
    cairo_t *texture_cr;
    glitz_surface_t *texture_surface;
    glitz_drawable_t *texture_drawable;
    glitz_format_t *format;
    glitz_drawable_format_t templ, *dformat;
    long mask;

    /* Create the surface and texture.  */
    format = glitz_find_standard_format (drawable, GLITZ_STANDARD_ARGB32);
    texture_surface = glitz_surface_create (drawable, format,
					    BG_WIDTH, BG_HEIGHT, 0, NULL);

    /* Now associate it to a texture.  */
    face_texture = glitz_texture_object_create (texture_surface);
    glitz_surface_destroy (texture_surface);

    /* Since for extra coolness we draw on the surface, we need a drawable
       too.  */
    templ.color        = format->color;
    mask =
        GLITZ_FORMAT_RED_SIZE_MASK     |
        GLITZ_FORMAT_GREEN_SIZE_MASK   |
        GLITZ_FORMAT_BLUE_SIZE_MASK    |
        GLITZ_FORMAT_ALPHA_SIZE_MASK;

    dformat = glitz_find_drawable_format (drawable, mask, &templ, 0);
    texture_drawable = glitz_create_drawable (drawable, dformat,
					      BG_WIDTH, BG_HEIGHT);
    glitz_surface_attach (texture_surface, texture_drawable,
			  GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);
    glitz_drawable_destroy (texture_drawable);

    /* And everything else that the COMP demo needs.  */
    cairo_texture_surface = cairo_glitz_surface_create (texture_surface);
    glitz_surface_destroy (texture_surface);
    texture_cr = cairo_create (cairo_texture_surface);
    cairo_surface_destroy (cairo_texture_surface);

    cairo_set_tolerance (texture_cr, 0.5);
    comp_setup (texture_cr, BG_WIDTH, BG_HEIGHT);

    /* Create the context for our own graphics.  */
    cube_context = glitz_context_create (drawable, glitz_drawable_get_format (drawable));
    glitz_context_make_current (cube_context, drawable);

    /* And initialize it with a display list.  */
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glitz_context_bind_texture (cube_context, face_texture);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glColor4f (1.0, 0.5, 0.0, 1.0);
    draw_cube ();
    glitz_context_unbind_texture (cube_context, face_texture);
    glEndList();

    glClearColor (0.0, 0.0, 0.0, 1.0);
}

double t = 0.0;
double angle_x = 0.0;
double angle_y = 1.0;
double angle_z = 2.0;

void
cube_render (int w, int h)
{
    comp_render (BG_WIDTH, BG_HEIGHT);
    glFinish ();

    t += 0.01745;
    angle_x += sin (t) * sin (t);
    angle_y += cos (t) * cos (t);
    angle_z += 0.1745;

    glitz_context_make_current (cube_context, drawable);

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective (40.0, w / (float) h, Z_OFS - CUBE_SIZE, Z_OFS + CUBE_SIZE);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    gluLookAt (0.0, 0.0, -Z_OFS, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glRotatef (angle_x, angle_y, angle_z, 1.0);
    glCallList (list);
    glFlush ();
}
