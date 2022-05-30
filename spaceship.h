#ifndef SPACESHIP_H_INCLUDED
#define SPACESHIP_H_INCLUDED

extern ball spaceship;

extern void spaceship_init_state ();
extern void spaceship_update_state ();
extern void spaceship_control (double dx, double dy);
extern void spaceship_draw (cairo_t * cr);

#endif
