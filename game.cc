#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "game.h"

double delta = DEFAULT_DELTA;	/* seconds */

int width = DEFAULT_WIDTH;
int height = DEFAULT_HEIGHT;

/* Graphics System
 */
GtkWidget * canvas;

