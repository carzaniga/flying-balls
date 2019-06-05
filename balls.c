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

static double delta = 0.01;

static unsigned int width = 0;
static unsigned int height = 0;

static unsigned int radius_min = 5;
static unsigned int radius_max = 10;

static unsigned int v_max = 100;
static unsigned int v_min = 0;

struct ball * balls = 0;
unsigned int n_balls = 50;

static double g_y = 20;
static double g_x = 0;

void balls_init_state () {
    srand(time(NULL));
    static const unsigned int border = 10;
    unsigned int w = width < 2*border ? 1 : width - 2*border;
    unsigned int h = height < 2*border ? 1 : height - 2*border;

    for (unsigned int i = 0; i < n_balls; ++i) {
	balls[i].x = border + rand() % w;
	balls[i].y = border + rand() % h;
	balls[i].v_x = v_min + rand() % (v_max + 1 - v_min);
	balls[i].v_y = v_min + rand() % (v_max + 1 - v_min);
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

static GdkPixbuf * pixbuf = 0;
static double clear_factor [3] = { 0.0, 0.0, 0.0 };

static void destroy_surface () {
    if (pixbuf)
	g_object_unref(pixbuf);

    pixbuf = 0;
}

static void create_surface (int w, int h) {
    destroy_surface();
    width = w;
    height = h;

    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8, width, height);
    assert(pixbuf);

    guchar * pixels = gdk_pixbuf_get_pixels(pixbuf);
    int row_stride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    for(int y = 0; y < height; ++y) {
	unsigned char * px = pixels;
	for(int x = 0; x < width; ++x) {
	    for(int i = 0; i < n_channels; ++i)
		*px++ = 0;
	}
	pixels += row_stride;
    }
}

static void update_state () {
    for(int i = 0; i < n_balls; ++i)
	for(int j = i + 1; j < n_balls; ++j)
	    ball_collision(balls + i, balls + j);

    for(int i = 0; i < n_balls; ++i)
	ball_update_state(balls + i);
}

static void do_draw (GtkWidget * widget) {
    guchar * const pixels = gdk_pixbuf_get_pixels(pixbuf);
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int row_stride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    /* clear pixmap */
    for(int y = 0; y < height; ++y) {
	unsigned char * px = pixels + y*row_stride;
	for(int x = 0; x < width; ++x) {
	    for(int i = 0; i < n_channels; ++i)
		*px++ *= clear_factor[i];
	}
    }

    /* draw balls */
    for(int i = 0; i < n_balls; ++i) {
	int x0 = balls[i].x <= balls[i].radius ? 0 : balls[i].x - balls[i].radius;
	int x1 = balls[i].x + balls[i].radius + 1 >= width ? width : balls[i].x + balls[i].radius + 1;
	int y0 = balls[i].y <= balls[i].radius ? 0 : balls[i].y - balls[i].radius;
	int y1 = balls[i].y + balls[i].radius + 1 >= height ? height : balls[i].y + balls[i].radius + 1;
	for (int y = y0; y < y1; ++y) {
	    assert(y >= 0);
	    unsigned char * px = pixels + y*row_stride + x0*n_channels;
	    int radius2_dy2 = balls[i].radius*balls[i].radius - (y - balls[i].y)*(y - balls[i].y);
	    for (int x = x0; x < x1; ++x) {
		if ((x - balls[i].x)*(x - balls[i].x) <= radius2_dy2)
		    for(int c = 0; c < n_channels; ++c)
			*px++ = balls[i].rgb_channels[c];
		else
		    px += n_channels;
	    }
	}
    }
    
    gdk_draw_pixbuf(widget->window, NULL, pixbuf,
		    0, 0, 0, 0, width, height,
		    GDK_RGB_DITHER_NONE, 0, 0);
}

static gint resize_pixbuf (GtkWidget *widget, GdkEventConfigure * event) {
    if (width == widget->allocation.width && height == widget->allocation.height)
	return FALSE;

    create_surface(widget->allocation.width, widget->allocation.height);
    do_draw(widget);
    return TRUE;
}

static gint keyboard_input (GtkWidget *widget, GdkEventKey *event) {
    if (event->type != GDK_KEY_PRESS)
	return FALSE;
    switch(event->keyval) {
    case GDK_KEY_Q:
    case GDK_KEY_q:
	gtk_main_quit();
	break;
    default:
	return FALSE;
    }
    return TRUE;
}

static gboolean expose_event (GtkWidget *widget, GdkEventExpose *event) {
    do_draw(widget);
    return TRUE;
}

static void destroy_window (void) {
    gtk_main_quit();
}

void print_usage (const char * progname) {
    fprintf(stderr,
	    "usage: %s [<width>x<height>] [n=<bumber of balls>] [clear=<clear-red>,<clear-green>,<clear-blue>] [fx=<x-force>] [fy=<y-force>] [radius=<min-radius>-<max-radius>] [delta=<frame-delta-time>]\n",
	    progname);
}

gboolean timeout (gpointer user_data) {
    guint64 start = g_get_monotonic_time ();

    GtkWidget * window = user_data;

    update_state();
    do_draw(window);

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
	if (sscanf(argv[i], "clear=%lf,%lf,%lf", clear_factor, clear_factor + 1, clear_factor + 2) == 3)
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
    
    create_surface(w, h);
    balls_init_state();

    gtk_init(0, 0);

    GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_resize(GTK_WINDOW(window), w, h);

    gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(destroy_window), NULL);
    gtk_signal_connect(GTK_OBJECT(window), "expose_event", (GtkSignalFunc) expose_event, NULL);
    gtk_signal_connect(GTK_OBJECT(window), "configure_event", (GtkSignalFunc) resize_pixbuf, NULL);
    gtk_signal_connect(GTK_OBJECT(window), "key_press_event", (GtkSignalFunc) keyboard_input, NULL);
    gtk_widget_set_events (window, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK);

    g_timeout_add (delta * 1000, timeout, window);

    gtk_widget_show_all(window);

    gtk_main();

    free(balls);
    destroy_surface();
    return 0;
}
