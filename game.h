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

#if 0
extern void game_init (); 	/* implemented by the application */

/* game frame function to be provided by the game application */
extern gboolean game_draw_frame (GtkWidget * widget, cairo_t * cr, gpointer data);

extern void set_game_resize_callback (gboolean (*game_resize) ());
extern void set_game_keyboard_input_callback (gint (*keyboard_input)(GtkWidget *, GdkEventKey *));
extern void set_game_mouse_scroll_callback (gboolean (*mouse_scroll)(GtkWidget *, GdkEvent *, gpointer));
#endif
#endif
