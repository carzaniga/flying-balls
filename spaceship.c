#include <math.h>
#include <gtk/gtk.h>

#include "balls.h"
#include "game.h"

struct ball spaceship;
double spaceship_thrust = 0;
int spaceship_thrust_countdown = 0;
int spaceship_thrust_init = 50;

void spaceship_init_state () {
    spaceship.x = width/2;
    spaceship.y = height/2;
    spaceship.radius = 30;
    spaceship.v_x = 0;
    spaceship.v_y = 0;
    spaceship.angle = 0;
    spaceship.v_angle = 0;
}

void spaceship_update_state () {
    if (spaceship_thrust > 0) {
	double fx = cos(spaceship.angle)*spaceship_thrust*4.0;
	double fy = sin(spaceship.angle)*spaceship_thrust*4.0;

	spaceship.x += delta*delta*fx/2.0;
	spaceship.v_x += delta*fx;
	spaceship.y += delta*delta*fy/2.0;
	spaceship.v_y += delta*fy;
	if (spaceship_thrust_countdown > 0)
	    --spaceship_thrust_countdown;
	else
	    spaceship_thrust = 0;
    }
    ball_update_state(&spaceship);
}

void spaceship_draw (cairo_t * cr) {
    static const double one_over_sqrt_2 = 0.70710678118654752440;
    cairo_save(cr);
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
    cairo_translate(cr, spaceship.x, spaceship.y);
    cairo_rotate(cr, spaceship.angle);
    cairo_arc(cr, 0, 0, spaceship.radius, 0, 2*M_PI);
    cairo_stroke(cr);
    cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, -one_over_sqrt_2*spaceship.radius, one_over_sqrt_2*spaceship.radius);
    cairo_line_to (cr, spaceship.radius, 0);
    cairo_line_to (cr, -one_over_sqrt_2*spaceship.radius, -one_over_sqrt_2*spaceship.radius);
    cairo_line_to (cr, 0, 0);
    cairo_stroke(cr);
    cairo_set_source_rgba(cr, 1.0, 0.5, 0.0, 1.0);
    for (unsigned int i = 0; i < spaceship_thrust; i += 5) {
	double d_angle = spaceship.radius/(spaceship.radius + 0.1*i)*0.25*M_PI*(1 - 0.99*i/spaceship_thrust);
	cairo_set_source_rgba(cr, 1.0, 1.0*(1 - 0.5*i/spaceship_thrust), 0.0, 1.0);
	cairo_arc(cr, 0, 0, spaceship.radius + i, M_PI - d_angle, M_PI + d_angle);
	cairo_stroke(cr);
	if (d_angle > 0.05)
	    d_angle = 0.7*d_angle;
    }
    cairo_restore(cr);
}

void spaceship_control (double dx, double dy) {
    spaceship.angle -= dx/4;
    if (spaceship.angle < 0)
	spaceship.angle += 2*M_PI;
    else if (spaceship.angle > 2*M_PI)
	spaceship.angle -= 2*M_PI;
    spaceship_thrust += dy;
    if (spaceship_thrust > 0)
	spaceship_thrust_countdown = spaceship_thrust_init;
    else
	spaceship_thrust = 0;
}
