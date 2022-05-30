#include <math.h>
#include <gtk/gtk.h>

#include "gravity.h"
#include "game.h"

static double g_y = 20;
static double g_x = 0;

static double g_r = 50;
static double g_g = 10000000;

static int constant_field = 1;

static int gravity_vector_countdown = 0;
static int gravity_vector_init = 300;

void gravity_constant_field (double x, double y) {
    constant_field = 1;
    g_x = x;
    g_y = y;
}

void gravity_newton_field (double r, double g) {
    constant_field = 0;
    g_r = r;
    g_g = g;
}

void gravity_draw (cairo_t * cr) {
    if (constant_field) {
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
    } else {
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_save (cr);
	cairo_new_path (cr);
	cairo_arc (cr, width/2, height/2, g_r, 0, 2*M_PI);
	cairo_set_line_width(cr, 3.0);
	cairo_stroke(cr);
	cairo_restore(cr);
    }
}

void gravity_show () {
    if (constant_field)
	gravity_vector_countdown = gravity_vector_init;
};

void gravity_change (double dx, double dy) {
    if (constant_field) {
	g_x += dx;
	g_y += dy;
	gravity_show ();
    } else {
	g_r += dx;
	g_g += dy;
    }
}

void gravity_get_vector (struct gravity_vector * v, const struct ball * b) {
    if (constant_field) {
	v->x = g_x;
	v->y = g_y;
    } else {
	double dx = width/2 - b->x;
	double dy = height/2 - b->y;
	double r2 = dx*dx+dy*dy;
	if (r2 < g_r*g_r) {
	    v->x = 0;
	    v->y = 0;
	} else {
	    double r = sqrt(r2);
	    v->x = g_g/r2/r*dx;
	    v->y = g_g/r2/r*dy;
	}
    }
}

void gravity_collisions (struct ball * begin, struct ball * end) {
    if (constant_field)
	return;
    for (struct ball * b = begin; b != end; ++b) {
	double dx = b->x - width/2;
	double dy = b->y - height/2;
	double d2 = dx*dx + dy*dy;
	double r = b->radius + g_r;
	if (d2 <= r*r) {
	    double dv_x = b->v_x;
	    double dv_y = b->v_y;

	    double f = dv_x*dx + dv_y*dy;

	    if (f < 0) {
		f /= d2;
		b->v_x -= 2*f*dx;
		b->v_y -= 2*f*dy;
	    }
	}
    }
}
