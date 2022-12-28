#ifndef BALLS_H_INCLUDED
#define BALLS_H_INCLUDED

#include <gtk/gtk.h>

#include "vec2d.h"

class ball_face;

class ball {
public:
    unsigned int radius;
    vec2d position;
    vec2d velocity;

    double angle;
    double v_angle;

    ball_face * face;

    void draw (cairo_t * cr) const;
};

extern ball * balls;
extern unsigned int n_balls;

extern unsigned int radius_min;
extern unsigned int radius_max;

extern unsigned int v_max;
extern unsigned int v_min;

extern unsigned int v_angle_min;
extern unsigned int v_angle_max;

extern const char * face_filename;
extern int face_rotation;

extern void balls_init ();
extern void balls_destroy ();
extern void ball_update_state (ball * p);
extern void ball_ball_collision (ball * p, ball * q);
extern void ball_reposition (ball * b);
extern void balls_draw (cairo_t * cr);

extern void restitution_coefficient_draw (cairo_t * cr);
extern void restitution_coefficient_set (double c);
extern double restitution_coefficient_get ();
extern void restitution_coefficient_change (double d);

#endif
