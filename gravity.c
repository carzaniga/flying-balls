#include <math.h>
#include <gtk/gtk.h>

#include "gravity.h"
#include "game.h"

double g_y = 20;
double g_x = 0;

static int gravity_vector_countdown = 0;
static int gravity_vector_init = 300;

void gravity_draw (cairo_t * cr) {
    if (gravity_vector_countdown != 0) {
	cairo_save(cr);
	cairo_new_path(cr);
	cairo_move_to(cr, width/2, height/2);
	cairo_line_to(cr, width/2 + g_x, height/2 + g_y);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_set_line_width(cr, 1.0);
	cairo_stroke(cr);
	cairo_arc(cr, width/2 + g_x, height/2 + g_y, 3, 0, 2*M_PI);
	cairo_fill(cr);
	if (gravity_vector_countdown > 0)
	    --gravity_vector_countdown;
	cairo_restore(cr);
    }
}

void gravity_show () {
    gravity_vector_countdown = gravity_vector_init;
};

void gravity_change (double dx, double dy) {
    g_x += dx;
    g_y += dy;
    gravity_show ();
};

