#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <gtk/gtk.h>

/* simulation/game framework
 */

extern double delta;		/* simulation time delta in seconds */

#define DEFAULT_DELTA 0.01

extern int width;	/* game canvas width */
extern int height;	/* game canvas height */

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 800

extern GtkWidget * canvas;	/* game canvas object */

#endif
