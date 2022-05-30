#ifndef BALLS_H_INCLUDED
#define BALLS_H_INCLUDED

#include <gtk/gtk.h>

class ball_face;

class ball {
public:
    double x;
    double y;
    unsigned int radius;

    double v_x;
    double v_y;

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
extern void ball_elastic_collision (ball * p, ball * q);
extern void ball_reposition (ball * b);
extern void balls_draw (cairo_t * cr);

#endif
