#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include <gdk/gdkkeysyms.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 800

struct ball {
    double x;
    double y;
    unsigned int radius;

    double v_x;
    double v_y;

    guchar rgb_channels[3];
};

static double delta = 0.01;	/* seconds */

static unsigned int width = DEFAULT_WIDTH;
static unsigned int height = DEFAULT_HEIGHT;

static unsigned int radius_min = 5;
static unsigned int radius_max = 10;

static unsigned int v_max = 100;
static unsigned int v_min = 0;

struct ball * balls = 0;
unsigned int n_balls = 50;

static double g_y = 20;
static double g_x = 0;

void random_velocity(struct ball * p) {
    double r2;
    do {
	p->v_x = v_min + rand() % (v_max + 1 - v_min);
	p->v_y = v_min + rand() % (v_max + 1 - v_min);
	r2 = p->v_x*p->v_x + p->v_y*p->v_y;
    } while (r2 > v_max*v_max || r2 < v_min*v_min);
}

void balls_init_state () {
    srand(time(NULL));
    static const unsigned int border = 10;
    unsigned int w = width < 2*border ? 1 : width - 2*border;
    unsigned int h = height < 2*border ? 1 : height - 2*border;

    for (unsigned int i = 0; i < n_balls; ++i) {
	balls[i].x = border + rand() % w;
	balls[i].y = border + rand() % h;
	random_velocity(balls + i);
	if (rand() % 2)
	    balls[i].v_x = -balls[i].v_x;
	if (rand() % 2)
	    balls[i].v_y = -balls[i].v_y;
	balls[i].radius = radius_min + rand() % (radius_max + 1 - radius_min);
	balls[i].rgb_channels[0] = rand() % 256;
	balls[i].rgb_channels[1] = rand() % 256;
	balls[i].rgb_channels[2] = rand() % 256;
    }
}

static void ball_collision (struct ball * p, struct ball * q) {
    double dx = q->x - p->x;
    double dy = q->y - p->y;
    double d2 = dx*dx + dy*dy;
    double r = p->radius + q->radius;
    if (d2 <= r*r) {
	double dv_x = q->v_x - p->v_x;
	double dv_y = q->v_y - p->v_y;

	double mp = p->radius * p->radius;
	double mq = q->radius * q->radius;

	double f = dv_x*dx + dv_y*dy;

	if (f < 0) {
	    f /= d2*(mp + mq);
	    p->v_x += 2*mq*f*dx;
	    p->v_y += 2*mq*f*dy;

	    q->v_x -= 2*mp*f*dx;
	    q->v_y -= 2*mp*f*dy;
	}
    }
}

static void ball_update_state (struct ball * p) {
    p->x += delta*p->v_x + delta*delta*g_x/2.0;
    p->v_x += delta*g_x;

    p->y += delta*p->v_y + delta*delta*g_y/2.0;
    p->v_y += delta*g_y;

    if (p->x + p->radius > width) {
	if (p->v_x > 0) {
	    p->x -= p->x + p->radius - width;
	    p->v_x = -p->v_x;
	}
    } else if (p->x < p->radius) {
	if (p->v_x < 0) {
	    p->x += p->radius - p->x;
	    p->v_x = -p->v_x;
	}
    } 

    if (p->y + p->radius > height) {
	if (p->v_y > 0) {
	    p->y -= p->y + p->radius - height;
	    p->v_y = -p->v_y;
	}
    } else if (p->y < p->radius) {
	if (p->v_y < 0) {
	    p->y += p->radius - p->y;
	    p->v_y = -p->v_y;
	}
    } 
}

static void update_state () {
    for(int i = 0; i < n_balls; ++i)
	for(int j = i + 1; j < n_balls; ++j)
	    ball_collision(balls + i, balls + j);

    for(int i = 0; i < n_balls; ++i)
	ball_update_state(balls + i);
}

static GtkWidget * window;
static cairo_t * cr = 0;

static void draw_gravity_vector() {
    cairo_new_path(cr);
    cairo_move_to(cr, width/2, height/2);
    cairo_line_to(cr, width/2 + g_x, height/2 + g_y);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 1.0);
    cairo_stroke(cr);
    cairo_arc(cr, width/2 + g_x, height/2 + g_y, 3, 0, 2*M_PI);
    cairo_fill(cr);
}

static void draw_balls_onto_window () {
    if (!cr)
	cr = gdk_cairo_create(window->window);

    /* clear pixmap */
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_paint(cr);

    draw_gravity_vector();

    /* draw balls */
    for(int i = 0; i < n_balls; ++i) {
	cairo_set_source_rgb(cr,
			     1.0*balls[i].rgb_channels[0]/255,
			     1.0*balls[i].rgb_channels[1]/255,
			     1.0*balls[i].rgb_channels[2]/255);
	cairo_arc(cr, balls[i].x, balls[i].y, balls[i].radius, 0, 2 * M_PI);
	cairo_fill(cr);
    }
}

static gint resize_event (GtkWidget *widget, GdkEventConfigure * event) {
    if (width == widget->allocation.width && height == widget->allocation.height)
	return FALSE;

    width = widget->allocation.width;
    height = widget->allocation.height;

    if (cr) {
        cairo_destroy(cr);
	cr = gdk_cairo_create(window->window);
    }

    draw_balls_onto_window();
    return TRUE;
}

static gint keyboard_input (GtkWidget *widget, GdkEventKey *event) {
    if (event->type != GDK_KEY_PRESS)
	return FALSE;
    switch(event->keyval) {
    case GDK_KEY_Up:
	g_y -= 10;
	break;
    case GDK_KEY_Down:
	g_y += 10;
	break;
    case GDK_KEY_Left:
	g_x -= 10;
	break;
    case GDK_KEY_Right:
	g_x += 10;
	break;
    case GDK_KEY_F:
    case GDK_KEY_f:
	g_x = rand() % 201 - 100;
	g_y = rand() % 201 - 100;
	break;
    case GDK_KEY_Q:
    case GDK_KEY_q:
	gtk_main_quit();
	break;
    default:
	return FALSE;
    }
    return TRUE;
}

static gboolean expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    return TRUE;
}

static void destroy_window (void) {
    gtk_main_quit();
    if (cr) {
	cairo_destroy(cr);
	cr = 0;
    }
}

void print_usage (const char * progname) {
    fprintf(stderr,
	    "usage: %s [options...]\n"
	    "options:\n"
	    "\t<width>x<height>\n"
	    "\tn=<bumber of balls>\n"
	    "\tfx=<x-force>\n"
	    "\tfy=<y-force>\n"
	    "\tradius=<min-radius>-<max-radius>\n"
	    "\tv=<min-velocity>-<max-velocity>\n"
	    "\tdelta=<frame-delta-time>\n",
	    progname);
}

gboolean timeout (gpointer user_data) {
    guint64 start = g_get_monotonic_time ();

    update_state();
    draw_balls_onto_window();

    guint64 elapsed_usec = g_get_monotonic_time () - start;

    static guint64 elapsed_usec_total = 0;
    static unsigned int samples = 0;
    if (samples == 30) {
	printf("\rtime for one frame: %lu usec (avg over %u samples)  ", elapsed_usec_total / samples, samples);
	fflush(stdout);
	samples = 0;
	elapsed_usec_total = 0;
    }
    ++samples;
    elapsed_usec_total += elapsed_usec;
    return TRUE;
}

int main (int argc, const char *argv[]) {
    int w = DEFAULT_WIDTH;
    int h = DEFAULT_HEIGHT;
    
    for (int i = 1; i < argc; ++i) {
	if (sscanf(argv[i], "%dx%d", &w, &h) == 2)
	    continue;
	if (sscanf(argv[i], "n=%u", &n_balls) == 1)
	    continue;
	if (sscanf(argv[i], "fx=%lf", &g_x) == 1)
	    continue;
	if (sscanf(argv[i], "fy=%lf", &g_y) == 1)
	    continue;
	if (sscanf(argv[i], "radius=%u-%u", &radius_min, &radius_max) == 2)
	    continue;
	if (sscanf(argv[i], "v=%u-%u", &v_min, &v_max) == 2)
	    continue;
	if (sscanf(argv[i], "delta=%lf", &delta) == 1)
	    continue;
	print_usage(argv[0]);
	return 1;
    }

    balls = malloc(sizeof(struct ball)*n_balls);
    assert(balls);
    
    balls_init_state();

    gtk_init(0, 0);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(window);
    gtk_window_set_default_size(GTK_WINDOW(window), w, h);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "Balls");

    g_signal_connect(window, "destroy", G_CALLBACK(destroy_window), NULL);
    g_signal_connect(window, "expose-event", G_CALLBACK(expose_event), NULL);
    g_signal_connect(window, "configure_event", G_CALLBACK(resize_event), NULL);
    g_signal_connect(window, "key_press_event", G_CALLBACK(keyboard_input), NULL);

    gtk_widget_set_events (window, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK);

    g_timeout_add (delta * 1000, timeout, window);

    gtk_widget_show_all(window);

    gtk_main();

    free(balls);

    return 0;
}
