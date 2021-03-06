#ifndef GRAVITY_H_INCLUDED
#define GRAVITY_H_INCLUDED

#include "balls.h"

struct gravity_vector {
    double x;
    double y;
};

extern void gravity_constant_field (double x, double y);
extern void gravity_newton_field (double r, double g);

extern void gravity_get_vector (struct gravity_vector * v, const struct ball * b);

extern void gravity_draw (cairo_t * cr);
extern void gravity_change (double dx, double dy);
extern void gravity_show ();

extern void gravity_collisions (struct ball * begin, struct ball * end);

#endif
