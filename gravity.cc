#include <math.h>
#include <gtk/gtk.h>

#include "gravity.h"
#include "game.h"

static vec2d g{.x = 0, .y = 20};

static double g_r = 50;
static double g_g = 10000000;

static int constant_field = 1;

static int gravity_vector_countdown = 0;
static int gravity_vector_init = 300;

void gravity_constant_field (double x, double y) {
    constant_field = 1;
    g.x = x;
    g.y = y;
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
	    cairo_line_to(cr, width/2 + g.x, height/2 + g.y);
	    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	    cairo_set_line_width(cr, 1.0);
	    cairo_stroke(cr);
	    cairo_arc(cr, width/2 + g.x, height/2 + g.y, 3, 0, 2*M_PI);
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
	g.x += dx;
	g.y += dy;
	gravity_show ();
    } else {
	g_r += dx;
	g_g += dy;
    }
}

vec2d gravity_vector (const ball * b) {
    if (constant_field) {
	return g;
    } else {
	vec2d b_c = vec2d{width/2.0,height/2.0} - b->position;
	double r2 = vec2d::dot(b_c,b_c);
	if (r2 < g_r*g_r) {
	    return vec2d{0,0};
	} else {
	    return g_g/r2/sqrt(r2)*b_c;
	}
    }
}

void gravity_collisions (ball * begin, ball * end) {
    if (constant_field)
	return;
    for (ball * b = begin; b != end; ++b) {
	vec2d b_c = b->position - vec2d{width/2.0,height/2.0};
	double d2 = vec2d::dot(b_c, b_c);
	double r = b->radius + g_r;
	if (d2 <= r*r) {
	    double f = vec2d::dot(b->velocity, b_c);
	    if (f < 0) {
		f /= d2;
		b->velocity -= 2*f*b_c;
	    }
	}
    }
}
