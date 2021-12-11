#ifndef GRAVITY_H_INCLUDED
#define GRAVITY_H_INCLUDED

extern double g_y;
extern double g_x;

extern void gravity_draw (cairo_t * cr);
extern void gravity_change (double dx, double dy);
extern void gravity_show ();

#endif
