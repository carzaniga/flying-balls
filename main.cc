#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 800

#include "game.h"
#include "balls.h"
#include "c_index.h"
#include "gravity.h"
#include "spaceship.h"

/* Trivial collision check
 */
void check_collisions_simple () {
    for(unsigned int i = 0; i < n_balls; ++i)
	for(unsigned int j = i + 1; j < n_balls; ++j)
	    ball_ball_collision(balls + i, balls + j);
    for(unsigned int j = 0; j < n_balls; ++j)
	ball_ball_collision(&spaceship, balls + j);
   gravity_collisions (balls, balls + n_balls);
   gravity_collisions (&spaceship, &spaceship + 1);
}

void check_collisions_with_index () {
    c_index_build();
    c_index_check_collisions(ball_ball_collision);
   for(unsigned int j = 0; j < n_balls; ++j)
	ball_ball_collision(&spaceship, balls + j);
   gravity_collisions (balls, balls + n_balls);
   gravity_collisions (&spaceship, &spaceship + 1);
}

void (*check_collisions)() = check_collisions_simple;

void update_state () {
    if (check_collisions)
	check_collisions();
    for(unsigned int i = 0; i < n_balls; ++i)
	ball_update_state(balls + i);
    spaceship_update_state();
}

/* Graphics System
 */

void game_init () {
    srand (time(NULL));
    balls_init ();
    assert(c_index_init());
    spaceship_init_state ();
}

void game_destroy () {
    c_index_destroy ();
    balls_destroy ();
}

static guint animation_timeout_id = 0;
gboolean animation_timeout (gpointer user_data);

static bool game_paused () {
    return animation_timeout_id == 0;
}

void game_animation_on_off () {
    if (animation_timeout_id == 0) {
	animation_timeout_id = g_timeout_add (delta * 1000, animation_timeout, canvas);
    } else {
	g_source_remove (animation_timeout_id);
	animation_timeout_id = 0;
    }
}

struct param_controls {
    void (*draw) (cairo_t * cr);
    gint (*keyboard_input) (GdkEventKey *event);
    gboolean (*mouse_scroll) (GtkWidget *widget, GdkEvent *event, gpointer user_data);
};

gint gravity_controls_keyboard_input (GdkEventKey *event) {
    switch(event->keyval) {
    case GDK_KEY_Up:
	gravity_change (0, -10);
	return TRUE;
    case GDK_KEY_Down:
	gravity_change (0, 10);
	return TRUE;
    case GDK_KEY_Left:
	gravity_change (-10, 0);
	return TRUE;
    case GDK_KEY_Right:
	gravity_change (10, 0);
	return TRUE;
    }
    return FALSE;
}

param_controls gravity_control = {
    .draw = gravity_draw,
    .keyboard_input = gravity_controls_keyboard_input,
};

gint restitution_coefficient_controls_keyboard_input (GdkEventKey *event) {
    switch(event->keyval) {
    case GDK_KEY_Up:
	restitution_coefficient_change (0.01);
	return TRUE;
    case GDK_KEY_Down:
	restitution_coefficient_change (-0.01);
	return TRUE;
    case GDK_KEY_Left:
	restitution_coefficient_change (-0.01);
	return TRUE;
    case GDK_KEY_Right:
	restitution_coefficient_change (0.01);
	return TRUE;
    }
    return FALSE;
}

param_controls restitution_coefficient_control = {
    .draw = restitution_coefficient_draw,
    .keyboard_input = restitution_coefficient_controls_keyboard_input,
};



static param_controls * param_control = nullptr;
static guint param_control_timeout_id = 0;
static const guint param_control_active_ms = 3000;

gboolean param_control_timeout (gpointer user_data) {
    param_control = nullptr;
    param_control_timeout_id = 0;
    gtk_widget_queue_draw(canvas);
    return FALSE;
}

void param_control_activate (struct param_controls * ctrl) {
    if (param_control) 
	g_source_remove (param_control_timeout_id);
    param_control = ctrl;
    param_control_timeout_id = g_timeout_add (param_control_active_ms, param_control_timeout, canvas);
    gtk_widget_queue_draw(canvas);
}

gboolean draw_frame (GtkWidget * widget, cairo_t *cr, gpointer data) {
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_paint(cr);
    if (param_control)
	param_control->draw (cr);
    gravity_draw_visible_field (cr);
    balls_draw (cr);
    spaceship_draw (cr);
    return FALSE;
}

gint configure_event (GtkWidget *widget, GdkEventConfigure * event) {
    if (width == event->width && height == event->height)
	return FALSE;

    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);

    for (ball * b = balls; b != balls + n_balls; ++b)
	ball_reposition (b);
    ball_reposition (&spaceship);
    return TRUE;
}

gint keyboard_input (GtkWidget *widget, GdkEventKey *event) {
    if (event->type != GDK_KEY_PRESS)
	return FALSE;
    if (param_control && param_control->keyboard_input (event)) {
	gtk_widget_queue_draw(canvas);
	g_source_remove (param_control_timeout_id);
	param_control_timeout_id = g_timeout_add (param_control_active_ms, param_control_timeout, canvas);
	return TRUE;
    }
    switch(event->keyval) {
    case GDK_KEY_R:
    case GDK_KEY_r:
	param_control_activate (&restitution_coefficient_control);
	return TRUE;
    case GDK_KEY_G:
    case GDK_KEY_g:
	param_control_activate (&gravity_control);
	return TRUE;
    case GDK_KEY_P:
    case GDK_KEY_p:
	game_animation_on_off ();
	return TRUE;
    case GDK_KEY_Q:
    case GDK_KEY_q:
	gtk_main_quit();
	return TRUE;
    }
    return FALSE;
}

gboolean mouse_scroll (GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    if (event->type == GDK_SCROLL && ! game_paused ()) {
        GdkEventScroll * e = (GdkEventScroll*) event;
	switch (e->direction) {
	case GDK_SCROLL_SMOOTH: {
	    double dx, dy;
	    gdk_event_get_scroll_deltas (event, &dx, &dy);
	    spaceship_control (dx, dy);
	    break;
	}
	default:
	    break;
	}
    }
    return TRUE;
}

void destroy_window (void) {
    gtk_main_quit();
}

void print_usage (const char * progname) {
    fprintf(stderr,
	    "usage: %s [options...]\n"
	    "options:\n"
	    "\t<width>x<height>\n"
	    "\tn=<number of balls>\n"
	    "\tfconst=<x-force>,<y-force> :: constant force field\n"
	    "\tfnewt=<radius>,<g> :: radial, Newtonian force field\n"
	    "\tfy=<y-force>\n"
	    "\tradius=<min-radius>-<max-radius>\n"
	    "\tv=<min-velocity>-<max-velocity>\n"
	    "\tdelta=<frame-delta-time> (in seconds)\n"
	    "\tface=<filename>\n"
	    "\tstats=<sample-count> :: rendering timing statitstics (0=disabled, default)\n"
	    "\tcollisions=<C> :: n=no collisions, s=simple, i=index\n"
	    "\trestitution=<C_r> :: restitution coefficient C_r\n"
	    "\t-r :: activate face rotation\n",
	    progname);
}

static unsigned int stats_sampling = 0;
static guint64 stats_update_usec = 0;
static unsigned int stats_update_samples = 0;
static guint64 stats_draw_usec = 0;
static unsigned int stats_draw_samples = 0;

gboolean draw_event (GtkWidget *widget, cairo_t * cr, gpointer data) {
    if (stats_sampling > 0) {
	guint64 start = g_get_monotonic_time ();
	draw_frame (widget, cr, data);
	stats_draw_usec += g_get_monotonic_time () - start;
	++stats_draw_samples;
    } else
	draw_frame (widget, cr, data);
    return FALSE;
}

gboolean animation_timeout (gpointer user_data) {
    if (stats_sampling > 0) {
	guint64 start = g_get_monotonic_time ();
	update_state();
	stats_update_usec += g_get_monotonic_time () - start;
	if (++stats_update_samples == stats_sampling) {
	    float uavg = 1.0*stats_update_usec / stats_update_samples;
	    float davg = 1.0*stats_draw_usec / stats_draw_samples;
	    printf("\rupdate = %.0f us, draw = %.0f us, load = %.0f%% (%u update, %u draw)  ",
		   uavg, davg, (uavg+davg)/(10000.0*delta),
		   stats_update_samples, stats_draw_samples);
	    fflush(stdout);
	    stats_update_usec = 0;
	    stats_update_samples = 0;
	    stats_draw_usec = 0;
	    stats_draw_samples = 0;
	}
    } else {
	update_state();
    }
    gtk_widget_queue_draw(canvas);
    return TRUE;
}

int main (int argc, const char *argv[]) {
    int w = DEFAULT_WIDTH;
    int h = DEFAULT_HEIGHT;
    double fa, fb;
    
    for (int i = 1; i < argc; ++i) {
	if (sscanf(argv[i], "%dx%d", &w, &h) == 2)
	    continue;
	if (sscanf(argv[i], "n=%u", &n_balls) == 1)
	    continue;
	if (sscanf(argv[i], "fconst=%lf,%lf", &fa, &fb) == 2) {
	    gravity_constant_field (fa,fb);
	    continue;
	}
	if (sscanf(argv[i], "fnewt=%lf,%lf", &fa, &fb) == 2) {
	    gravity_newton_field (fa,fb);
	    continue;
	}
	if (sscanf(argv[i], "radius=%u-%u", &radius_min, &radius_max) == 2)
	    continue;
	if (sscanf(argv[i], "v=%u-%u", &v_min, &v_max) == 2)
	    continue;
	if (sscanf(argv[i], "delta=%lf", &delta) == 1)
	    continue;
	if (strncmp(argv[i], "face=", 5) == 0) {
	    face_filename = argv[i] + 5;
	    continue;
	}
	if (sscanf(argv[i], "stats=%u", &stats_sampling) == 1)
	    continue;
	char collisions;
	if (sscanf(argv[i], "collisions=%c", &collisions) == 1) {
	    switch (collisions) {
	    case 'i':
	    case 'I':
		check_collisions = check_collisions_with_index;
		continue;
	    case '0':
	    case 'N':
	    case 'n':
		check_collisions = 0;
		continue;
	    case 's':
	    case 'S':
		check_collisions = check_collisions_simple;
		continue;
	    }
	}
	if (strcmp(argv[i], "-r") ==  0) {
	    face_rotation = 1;
	    continue;
	}
	double c_r;
	if (sscanf(argv[i], "restitution=%lf", &c_r) == 1) {
	    restitution_coefficient_set (c_r);
	    continue;
	}
	print_usage(argv[0]);
	return 1;
    }

    gtk_init(0, 0);

    GtkWidget * window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "Game");

    g_signal_connect(window, "destroy", G_CALLBACK(destroy_window), NULL);
    g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK(destroy_window), NULL);
    g_signal_connect(G_OBJECT (window), "key-press-event", G_CALLBACK(keyboard_input), NULL);
    gtk_widget_set_events (window, GDK_EXPOSURE_MASK | GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK | GDK_KEY_PRESS_MASK);

    canvas = gtk_drawing_area_new ();

    g_signal_connect (G_OBJECT (canvas), "configure-event", G_CALLBACK(configure_event), NULL);
    g_signal_connect (G_OBJECT (canvas), "draw", G_CALLBACK (draw_event), NULL);
    g_signal_connect (G_OBJECT (canvas), "scroll-event",G_CALLBACK(mouse_scroll), NULL);
    gtk_widget_set_events (canvas, GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);

    gtk_container_add (GTK_CONTAINER (window), canvas);

    game_animation_on_off ();

    gtk_widget_show_all(window);

    game_init ();

    gtk_main();

    if (stats_sampling > 0)
	printf("\n");

    game_destroy ();

    return 0;
}
