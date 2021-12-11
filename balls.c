#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 800

#include "balls.h"
#include "c_index.h"

double delta = 0.01;	/* seconds */

unsigned int width = DEFAULT_WIDTH;
unsigned int height = DEFAULT_HEIGHT;

unsigned int radius_min = 5;
unsigned int radius_max = 10;

unsigned int v_max = 100;
unsigned int v_min = 0;

unsigned int v_angle_min = 0;
unsigned int v_angle_max = 100;

struct ball * balls = 0;
unsigned int n_balls = 50;

double g_y = 20;
double g_x = 0;

void random_velocity(struct ball * p) {
    double r2;
    do {
	p->v_x = v_min + rand() % (v_max + 1 - v_min);
	p->v_y = v_min + rand() % (v_max + 1 - v_min);
	r2 = p->v_x*p->v_x + p->v_y*p->v_y;
    } while (r2 > v_max*v_max || r2 < v_min*v_min);
}

struct ball spaceship;
double spaceship_thrust = 0;
int spaceship_thrust_countdown = 0;
int spaceship_thrust_init = 50;

void spaceship_init_state () {
    spaceship.x = width/2;
    spaceship.y = height/2;
    spaceship.radius = 30;
    spaceship.v_x = 0;
    spaceship.v_y = 0;
    spaceship.angle = 0;
    spaceship.v_angle = 0;
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
	unsigned int v_angle_360 = (v_angle_min + rand() % (v_angle_max + 1 - v_angle_min)) % 360;
	balls[i].v_angle = 2*M_PI*v_angle_360/360;
	balls[i].angle = (rand() % 360)*2*M_PI/360;
    }
}

void ball_elastic_collision (struct ball * p, struct ball * q) {
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

void ball_update_state (struct ball * p) {
    p->x += delta*p->v_x + delta*delta*g_x/2.0;
    p->v_x += delta*g_x;

    p->y += delta*p->v_y + delta*delta*g_y/2.0;
    p->v_y += delta*g_y;

    if (p->x + p->radius > width) { /* right wall */
	if (p->v_x > 0) {
	    p->x -= p->x + p->radius - width;
	    p->v_x = -p->v_x;
	}
    } else if (p->x < p->radius) { /* left wall */
	if (p->v_x < 0) {
	    p->x += p->radius - p->x;
	    p->v_x = -p->v_x;
	}
    } 

    if (p->y + p->radius > height) { /* bottom wall */
	if (p->v_y > 0) {
	    p->y -= p->y + p->radius - height;
	    p->v_y = -p->v_y;
	}
    } else if (p->y < p->radius) { /* top wall */
	if (p->v_y < 0) {
	    p->y += p->radius - p->y;
	    p->v_y = -p->v_y;
	}
    } 
    p->angle += delta*p->v_angle;
    while (p->angle >= 2*M_PI)
	p->angle -= 2*M_PI;
    while (p->angle < 0)
	p->angle += 2*M_PI;
}

void spaceship_update_state () {
    if (spaceship_thrust > 0) {
	double fx = cos(spaceship.angle)*spaceship_thrust*4.0;
	double fy = sin(spaceship.angle)*spaceship_thrust*4.0;

	spaceship.x += delta*delta*fx/2.0;
	spaceship.v_x += delta*fx;
	spaceship.y += delta*delta*fy/2.0;
	spaceship.v_y += delta*fy;
	if (spaceship_thrust_countdown > 0)
	    --spaceship_thrust_countdown;
	else
	    spaceship_thrust = 0;
    }
    ball_update_state(&spaceship);
}

void reposition_within_borders () {
    for(int i = 0; i < n_balls; ++i) {
	struct ball * p = balls + i;
	if (p->x < p->radius)
	    p->x = p->radius;
	else if (p->x + p->radius > width)
	    p->x = width - p->radius;
	if (p->y < p->radius)
	    p->y = p->radius;
	else if (p->y + p->radius > height)
	    p->y = height - p->radius;
    }
}

void movement_and_borders () {
    for(int i = 0; i < n_balls; ++i)
	ball_update_state(balls + i);
    spaceship_update_state();
}

/* Trivial collision check
 */

void check_collisions_simple () {
    for(int i = 0; i < n_balls; ++i)
	for(int j = i + 1; j < n_balls; ++j)
	    ball_elastic_collision(balls + i, balls + j);
    for(int j = 0; j < n_balls; ++j)
	ball_elastic_collision(&spaceship, balls + j);
}

void check_collisions_with_index () {
    c_index_build();
    c_index_check_collisions(ball_elastic_collision);
}

void (*check_collisions)() = 0;

void update_state () {
    if (check_collisions)
	check_collisions();
    movement_and_borders();
}

/* Graphics System
 */

GtkWidget * canvas;

int gravity_vector_countdown = 0;
int gravity_vector_init = 300;

void draw_gravity_vector(cairo_t * cr) {
    if (gravity_vector_countdown != 0) {
	cairo_save(cr);
	cairo_new_path(cr);
	cairo_move_to(cr, width/2, height/2);
	cairo_line_to(cr, width/2 + g_x, height/2 + g_y);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_set_line_width(cr, 1.0);
	cairo_stroke(cr);
	cairo_arc(cr, width/2 + g_x, height/2 + g_y, 3, 0, 2*M_PI);
	cairo_fill(cr);
	if (gravity_vector_countdown > 0)
	    --gravity_vector_countdown;
	cairo_restore(cr);
    }
}

const char * face_filename = 0;
int face_rotation = 0;

static const double linear_rotation_unit = 2.0;

unsigned int faces_count;
struct ball_face ** faces;

struct ball_face {
    unsigned int rotations;
    cairo_surface_t ** c_faces;
};

static double random_color_component() {
    return 1.0*(rand() % 200 + 56)/255;
};

struct ball_face * new_ball_face(unsigned int radius, cairo_surface_t * face, int rotation) {
    struct ball_face * f = malloc(sizeof(struct ball_face));
    if (!f)
	return 0;
    if (face && rotation) {
	f->rotations = 2*M_PI * radius / linear_rotation_unit;
    } else {
	f->rotations = 1;
    }
    f->c_faces = malloc(sizeof(cairo_surface_t *)*f->rotations);
    if (!f->c_faces) {
	free(f);
	return 0;
    }
    for (int i = 0; i < f->rotations; ++i) {
	f->c_faces[i] = gdk_window_create_similar_surface(gtk_widget_get_window(canvas),
							  CAIRO_CONTENT_COLOR_ALPHA,
							  2*radius, 2*radius);
	assert(f->c_faces[i]);
	cairo_t * ball_cr = cairo_create(f->c_faces[i]);
	cairo_translate(ball_cr, radius, radius);
	cairo_arc(ball_cr, 0.0, 0.0, radius, 0, 2 * M_PI);
	cairo_clip(ball_cr);

	if (face) {
	    int face_x_offset = cairo_image_surface_get_width (face) / 2;
	    int face_y_offset = cairo_image_surface_get_height (face) / 2;
	    cairo_rotate(ball_cr, i*2*M_PI/f->rotations);
	    cairo_scale (ball_cr, 1.0 * radius / face_x_offset, 1.0 * radius / face_y_offset);
	    cairo_set_source_surface(ball_cr, face, -face_x_offset, -face_y_offset);
	    cairo_paint(ball_cr);
	} else {
	    cairo_pattern_t *pat;
	    pat = cairo_pattern_create_radial (-0.2*radius, -0.2*radius, 0.2*radius,
					       -0.2*radius, -0.2*radius, 1.3*radius);
	    double col_r = random_color_component();
	    double col_g = random_color_component();
	    double col_b = random_color_component();
	    cairo_pattern_add_color_stop_rgba (pat, 0, col_r, col_g, col_b, 1);
	    cairo_pattern_add_color_stop_rgba (pat, 1, col_r/3, col_g/3, col_b/3, 1);
	    cairo_set_source (ball_cr, pat);
	    cairo_arc (ball_cr, 0.0, 0.0, radius, 0, 2 * M_PI);
	    cairo_fill (ball_cr);
	}
	cairo_surface_flush(f->c_faces[i]);
	cairo_destroy(ball_cr);
    }
    return f;
}

void init_graphics() {
    cairo_surface_t * face_surface = 0;

    if (face_filename) {
	face_surface = cairo_image_surface_create_from_png (face_filename);
	if (cairo_surface_status(face_surface) != CAIRO_STATUS_SUCCESS) {
	    cairo_surface_destroy (face_surface);
	    face_surface = 0;
	    fprintf(stderr, "could not create surface from PNG file %s\n", face_filename);
	}
    }
    if (face_surface) {
	faces_count = radius_max + 1 - radius_min;
	faces = malloc(sizeof(struct ball_face *)*faces_count);
	for (unsigned int i = 0; i < faces_count; ++i)
	    faces[i] = 0;
	for(struct ball * b = balls; b != balls + n_balls; ++b) {
	    unsigned int r_idx = b->radius - radius_min;
	    if (!faces[r_idx])
		faces[r_idx] = new_ball_face(b->radius, face_surface, face_rotation);
	    b->face = faces[r_idx];
	}
	cairo_surface_destroy (face_surface);
    } else {
	faces_count = n_balls;
	faces = malloc(sizeof(struct ball_face *)*faces_count);
	for (unsigned int i = 0; i < n_balls; ++i)
	    balls[i].face = faces[i] = new_ball_face(balls[i].radius, 0, face_rotation);
    }
}

void destroy_graphics() {
    if (!faces)
	return;
    for (int i = 0; i < faces_count; ++i) {
	if (faces[i]) {
	    if (faces[i]->c_faces) {
		for (unsigned int j = 0; j < faces[i]->rotations; ++j)
		    cairo_surface_destroy(faces[i]->c_faces[j]);
		free(faces[i]->c_faces);
	    }
	    free(faces[i]);
	}
    }
    free(faces);
    faces = 0;
    faces_count = 0;
}

void draw_space_ship (cairo_t * cr) {
    static const double one_over_sqrt_2 = 0.70710678118654752440;
    cairo_save(cr);
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
    cairo_translate(cr, spaceship.x, spaceship.y);
    cairo_rotate(cr, spaceship.angle);
    cairo_arc(cr, 0, 0, spaceship.radius, 0, 2*M_PI);
    cairo_stroke(cr);
    cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, -one_over_sqrt_2*spaceship.radius, one_over_sqrt_2*spaceship.radius);
    cairo_line_to (cr, spaceship.radius, 0);
    cairo_line_to (cr, -one_over_sqrt_2*spaceship.radius, -one_over_sqrt_2*spaceship.radius);
    cairo_line_to (cr, 0, 0);
    cairo_stroke(cr);
    for (unsigned int i = 0; i < spaceship_thrust; i += 5) {
	cairo_arc(cr, 0, 0, spaceship.radius + i, 0.7*M_PI, 1.3*M_PI);
	cairo_stroke(cr);
    }
    cairo_restore(cr);
}

void draw_balls (cairo_t * cr) {
    for (const struct ball * b = balls; b != balls + n_balls; ++b) {
	cairo_save(cr);
	cairo_translate(cr, b->x - b->radius, b->y - b->radius);
	unsigned int face_id;
	if (b->face->rotations == 1)
	    face_id = 0;
	else {
	    face_id = b->face->rotations*b->angle/(2*M_PI);
	    assert(face_id < b->face->rotations);
	    if (face_id >= b->face->rotations)
		face_id %= b->face->rotations;
	}
	cairo_set_source_surface(cr, b->face->c_faces[face_id], 0, 0);
	cairo_paint(cr);
	cairo_restore(cr);
    }
}

gboolean draw_frame (GtkWidget * widget, cairo_t *cr, gpointer data) {
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_paint(cr);
    draw_gravity_vector(cr);
    draw_balls(cr);
    draw_space_ship(cr);
    return FALSE;
}

gint configure_event (GtkWidget *widget, GdkEventConfigure * event) {
    if (width == gtk_widget_get_allocated_width(widget) && height == gtk_widget_get_allocated_height(widget))
	return FALSE;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    reposition_within_borders();
    return TRUE;
}

gint keyboard_input (GtkWidget *widget, GdkEventKey *event) {
    if (event->type != GDK_KEY_PRESS)
	return FALSE;
    switch(event->keyval) {
    case GDK_KEY_Up:
	g_y -= 10;
	gravity_vector_countdown = gravity_vector_init;
	break;
    case GDK_KEY_Down:
	g_y += 10;
	gravity_vector_countdown = gravity_vector_init;
	break;
    case GDK_KEY_Left:
	g_x -= 10;
	gravity_vector_countdown = gravity_vector_init;
	break;
    case GDK_KEY_Right:
	g_x += 10;
	gravity_vector_countdown = gravity_vector_init;
	break;
    case GDK_KEY_G:
    case GDK_KEY_g:
	gravity_vector_countdown = gravity_vector_init;
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

gboolean mouse_scroll (GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    if (event->type == GDK_SCROLL) {
        GdkEventScroll * e = (GdkEventScroll*) event;
	switch (e->direction) {
	case GDK_SCROLL_SMOOTH: {
	    double dx, dy;
	    gdk_event_get_scroll_deltas (event, &dx, &dy);
	    spaceship.angle -= dx/4;
	    if (spaceship.angle < 0)
		spaceship.angle += 2*M_PI;
	    else if (spaceship.angle > 2*M_PI)
		spaceship.angle -= 2*M_PI;
	    spaceship_thrust += dy;
	    if (spaceship_thrust > 0)
		spaceship_thrust_countdown = spaceship_thrust_init;
	    else
		spaceship_thrust = 0;
	    break;
	}
	case GDK_SCROLL_LEFT:
	    g_x -= 10;
	    gravity_vector_countdown = gravity_vector_init;
	    break;
	case GDK_SCROLL_RIGHT:
	    g_x += 10;
	    gravity_vector_countdown = gravity_vector_init;
	    break;
	case GDK_SCROLL_UP:
	    g_y += 10;
	    gravity_vector_countdown = gravity_vector_init;
	    break;
	case GDK_SCROLL_DOWN:
	    g_y -= 10;
	    gravity_vector_countdown = gravity_vector_init;
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
	    "\tfx=<x-force>\n"
	    "\tfy=<y-force>\n"
	    "\tradius=<min-radius>-<max-radius>\n"
	    "\tv=<min-velocity>-<max-velocity>\n"
	    "\tdelta=<frame-delta-time> (in seconds)\n"
	    "\tface=<filename>\n"
	    "\tstats=<sample-count> :: rendering timing statitstics (0=disabled, default)\n"
	    "\tcollisions=<C> :: n=no collisions, s=simple, i=index\n"
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

gboolean timeout (gpointer user_data) {
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
	print_usage(argv[0]);
	return 1;
    }

    balls = malloc(sizeof(struct ball)*n_balls);
    assert(balls);

    assert(c_index_init());
    
    balls_init_state();
    spaceship_init_state();

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

    g_timeout_add (delta * 1000, timeout, canvas);

    gtk_widget_show_all(window);

    init_graphics();

    gtk_main();

    if (stats_sampling > 0)
	printf("\n");

    destroy_graphics();
    c_index_destroy();
    free(balls);

    return 0;
}
