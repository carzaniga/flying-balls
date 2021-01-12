#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 800

struct ball_face;

struct ball {
    double x;
    double y;
    unsigned int radius;

    double v_x;
    double v_y;

    double angle;
    double v_angle;

    struct ball_face * face;
};

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

double clear_alpha = 1.0;

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
	unsigned int v_angle_360 = (v_angle_min + rand() % (v_angle_max + 1 - v_angle_min)) % 360;
	balls[i].v_angle = 2*M_PI*v_angle_360/360;
	balls[i].angle = (rand() % 360)*2*M_PI/360;
    }
}

void ball_collision (struct ball * p, struct ball * q) {
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

#if 0
static void tangential_friction1(double u, double p, double r, double * v, double * q) {
    static const double a = 0.0;

    /*
     *                  2   2      2                 2   2  2
     *     sqrt((6 - 2 a ) u  + 4 a  p r u + (3 - 2 a ) p  r ) + a u - a p r
     * v = -----------------------------------------------------------------
     *                                     3
     *
     *                  2   2      2                 2   2  2
     *     sqrt((6 - 2 a ) u  + 4 a  p r u + (3 - 2 a ) p  r ) - 2 a u + 2 a p r
     * q = ---------------------------------------------------------------------
     *                                      3 r
     */
    double a2 = a*a;
    double u2 = u*u;
    double p2 = p*p;
    double r2 = r*r;
    double sr = sqrt((6 - 2*a2)*u2 + 4*a2*p*r*u + (3 - 2*a2)*p2*r2);
    *v = (sr + a*u - a*p*r)/3;
    *q = (sr - 2*a*u + 2*a*p*r)/(3*r);
}

static void tangential_friction2(double u, double p, double r, double * v, double * q) {
    static const double a = 1.0;

    /*
     *                  2   2      2                 2   2  2
     *     sqrt((6 - 2 a ) u  + 4 a  p r u + (3 - 2 a ) p  r ) + a u + a p r
     * v = -----------------------------------------------------------------
     *                                     3
     *
     *                  2   2      2                 2   2  2
     *     sqrt((6 - 2 a ) u  + 4 a  p r u + (3 - 2 a ) p  r ) - 2 a u - 2 a p r
     * q = ---------------------------------------------------------------------
     *                                      3 r
     */
    double a2 = a*a;
    double u2 = u*u;
    double p2 = p*p;
    double r2 = r*r;
    double sr = sqrt((6 - 2*a2)*u2 + 4*a2*p*r*u + (3 - 2*a2)*p2*r2);
    *v = (sr + a*u + a*p*r)/3;
    *q = (sr - 2*a*u - 2*a*p*r)/(3*r);
}
#endif
static void tangential_friction3(double u, double p, double r, double * v, double * q) {
    static const double a = 0.5;
    double w = u - p*r;
    *v = u - a*w;
#if 0
    *q = sqrt(p*p*r*r - 2*a*a*w + 4*u*a*w)/r;
    if (w > 0)
	*q = -*q;
#else
    *q = (*v)/r;
#endif
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
	    /* tangential friction */
#if 0
	    tangential_friction(p->v_y, -p->v_angle, p->radius, &(p->v_y), &(p->v_angle));
	    p->v_angle = -p->v_angle;
#endif
	}
    } else if (p->x < p->radius) { /* left wall */
	if (p->v_x < 0) {
	    p->x += p->radius - p->x;
	    p->v_x = -p->v_x;
	    /* tangential friction */
#if 0
	    tangential_friction(p->v_y, p->v_angle, p->radius, &(p->v_y), &(p->v_angle));
#endif
	}
    } 

    if (p->y + p->radius > height) { /* bottom wall */
	if (p->v_y > 0) {
	    p->y -= p->y + p->radius - height;
	    p->v_y = -p->v_y;
	    /* tangential friction */
#if 1
	    tangential_friction3(p->v_x, p->v_angle, p->radius, &(p->v_x), &(p->v_angle));
#endif
	}
    } else if (p->y < p->radius) { /* top wall */
	if (p->v_y < 0) {
	    p->y += p->radius - p->y;
	    p->v_y = -p->v_y;
	    /* tangential friction */
#if 0
	    tangential_friction(p->v_x, -p->v_angle, p->radius, &(p->v_x), &(p->v_angle));
	    p->v_angle = -p->v_angle;
#endif
	}
    } 
    p->angle += delta*p->v_angle;
    while (p->angle >= 2*M_PI)
	p->angle -= 2*M_PI;
    while (p->angle < 0)
	p->angle += 2*M_PI;
}

void movement_and_borders () {
    for(int i = 0; i < n_balls; ++i)
	ball_update_state(balls + i);
}

/* Collision check with index
*/
struct rectangle {
    double min_x;			/* left */
    double min_y;			/* bottom */
    double max_x;			/* right */
    double max_y;			/* top */
};

struct bt_node {
    struct ball * ball;
    struct rectangle r;
    struct bt_node * left;
    struct bt_node * right;
};

struct bt_node * c_index = 0;

static struct bt_node * c_index_init_node(struct bt_node * n, struct ball * b) {
    n->ball = b;
    n->r.min_x = b->x - b->radius;
    n->r.min_y = b->y - b->radius;
    n->r.max_x = b->x + b->radius;
    n->r.max_y = b->y + b->radius;
    n->left = 0;
    n->right = 0;
    return n;
}

static void c_index_add_ball(struct bt_node * n, const struct ball * b) {
    if (n->r.min_x > b->x - b->radius)
	n->r.min_x = b->x - b->radius;
    if (n->r.min_y > b->y - b->radius)
	n->r.min_y = b->y - b->radius;
    if (n->r.max_x < b->x + b->radius)
	n->r.max_x = b->x + b->radius;
    if (n->r.max_y < b->y + b->radius)
	n->r.max_y = b->y + b->radius;
}

static void c_index_insert(struct bt_node * t, struct bt_node * n, struct ball * b) {
    double w = width;
    double h = height;
    double ref_x = 0.0;
    double ref_y = 0.0;
    c_index_init_node(n, b);
    for (;;) {
	c_index_add_ball(t, b);
	if (w > h) { /* horizontal split */
	    if (b->x <= t->ball->x) {
		if (t->left) {
		    w = t->ball->x - ref_x;
		    t = t->left;
		} else {
		    t->left = n;
		    return;
		}
	    } else {
		if (t->right) {
		    w -= t->ball->x - ref_x;
		    ref_x = t->ball->x;
		    t = t->right;
		} else {
		    t->right = n;
		    return;
		}
	    }
	} else {		/* vertical split */
	    if (b->y <= t->ball->y) {
		if (t->left) {
		    h = t->ball->y - ref_y;
		    t = t->left;
		} else {
		    t->left = n;
		    return;
		}
	    } else {
		if (t->right) {
		    h -= t->ball->y - ref_y;
		    ref_y = t->ball->y;
		    t = t->right;
		} else {
		    t->right = n;
		    return;
		}
	    }
	}
    }
}

void c_index_build() {
    c_index_init_node(c_index, balls);
    for(int i = 1; i < n_balls; ++i)
	c_index_insert(c_index, c_index + i, balls + i);
}

struct bt_node ** c_index_stack = 0;
unsigned int c_index_stack_top = 0;

static void c_index_stack_clear() {
    c_index_stack_top = 0;
}

static void c_index_stack_push(struct bt_node * n) {
    c_index_stack[c_index_stack_top++] = n;
}

static struct bt_node * c_index_stack_pop() {
    if (c_index_stack_top > 0)
	return c_index_stack[--c_index_stack_top];
    else
	return 0;
}

static int c_index_ball_in_rectangle(const struct bt_node * n, const struct ball * b) {
    return n->r.min_x <= b->x + b->radius
	&& n->r.max_x >= b->x - b->radius
	&& n->r.min_y <= b->y + b->radius
	&& n->r.max_y >= b->y - b->radius;
}

static int c_index_must_check(const struct bt_node * n, const struct ball * b) {
    return n != 0 && n->ball < b && c_index_ball_in_rectangle(n, b);
}

void c_index_check_collisions() {
    for(struct ball * b = balls + 1; b < balls + n_balls; ++b) {
	c_index_stack_clear();
	struct bt_node * n = c_index;
	do {
	    ball_collision(n->ball, b);
	    if (c_index_must_check(n->left, b)) {
		if (c_index_must_check(n->right, b))
		    c_index_stack_push(n->right);
		n = n->left;
	    } else if (c_index_must_check(n->right, b)) {
		n = n->right;
	    } else {
		n = c_index_stack_pop();
	    }
	} while (n);
    }
}

int c_index_init() {
    if (!c_index)
	c_index = malloc(sizeof(struct bt_node) * n_balls);
    if (!c_index)
	return 0;
    if (!c_index_stack)
	c_index_stack = malloc(sizeof(struct bt_node *) * n_balls);
    if (!c_index_stack)
	return 0;
    return 1;
}

void c_index_destroy() {
    if (c_index)
	free(c_index);
    if (c_index_stack)
	free(c_index_stack);
    c_index = 0;
    c_index_stack = 0;
}

/* Trivial collision check
 */

void check_collisions_simple () {
    for(int i = 0; i < n_balls; ++i)
	for(int j = i + 1; j < n_balls; ++j)
	    ball_collision(balls + i, balls + j);
}

void check_collisions_with_index () {
    c_index_build();
    c_index_check_collisions();
}

void (*check_collisions)() = 0;

void update_state () {
    if (check_collisions)
	check_collisions();
    movement_and_borders();
}

/* Graphics System
 */

GtkWidget * window;
GtkWidget * canvas;

int gravity_vector_countdown = 0;
int gravity_vector_init = 300;

void draw_gravity_vector(cairo_t * cr) {
    if (gravity_vector_countdown != 0) {
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
	f->c_faces[i] = gdk_window_create_similar_surface(gtk_widget_get_window(window),
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
	    cairo_set_source_rgb(ball_cr, 1.0*(rand() % 256)/255, 1.0*(rand() % 256)/255, 1.0*(rand() % 256)/255);
	    cairo_paint(ball_cr);
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

void draw_balls_onto_window () {
    /* clear pixmap */
    GdkWindow * window = gtk_widget_get_window(canvas);
    cairo_region_t * c_region = cairo_region_create();
    GdkDrawingContext * d_context = gdk_window_begin_draw_frame (window, c_region);

    cairo_t * cr = gdk_drawing_context_get_cairo_context (d_context);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, clear_alpha);
    cairo_paint(cr);

    draw_gravity_vector(cr);

    /* draw balls */
    for(const struct ball * b = balls; b != balls + n_balls; ++b) {
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
    gdk_window_end_draw_frame(window, d_context);
    cairo_region_destroy(c_region);
}

gint configure_event (GtkWidget *widget, GdkEventConfigure * event) {
    if (width == gtk_widget_get_allocated_width(widget) && height == gtk_widget_get_allocated_height(widget))
	return FALSE;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

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

gboolean expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    draw_balls_onto_window();
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
	    "\tdelta=<frame-delta-time>\n"
	    "\tface=<filename>\n"
	    "\tclear=<clear-alpha>\n"
	    "\tstats=<sample-count> :: rendering timing statitstics (0=disabled, default)\n"
	    "\tcollisions=<C> :: n=no collisions, s=simple, i=index\n"
	    "\t-r :: activate face rotation\n",
	    progname);
}

unsigned int stats_sampling = 0;

gboolean timeout (gpointer user_data) {
    guint64 start = 0, elapsed_usec;
    if (stats_sampling > 0)
	start = g_get_monotonic_time ();

    update_state();
    draw_balls_onto_window();

    if (stats_sampling > 0) {
	elapsed_usec = g_get_monotonic_time () - start;

	static guint64 elapsed_usec_total = 0;
	static unsigned int samples = 0;
	if (samples == stats_sampling) {
	    printf("\rframe rendering: time = %lu usec, max freq = %.2f (avg over %u samples)  ", elapsed_usec_total / samples, (1000000.0 * samples) / elapsed_usec_total, samples);
	    fflush(stdout);
	    samples = 0;
	    elapsed_usec_total = 0;
	}
	++samples;
	elapsed_usec_total += elapsed_usec;
    }
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
	if (sscanf(argv[i], "clear=%lf", &clear_alpha) == 1)
	    continue;
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

    gtk_init(0, 0);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "Game");

    g_signal_connect(window, "destroy", G_CALLBACK(destroy_window), NULL);
    g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK(destroy_window), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(keyboard_input), NULL);
    gtk_widget_set_events (window, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK);

    canvas = gtk_drawing_area_new ();
    g_signal_connect (G_OBJECT (canvas), "expose-event", G_CALLBACK (expose_event), NULL);
    g_signal_connect (G_OBJECT (canvas), "configure-event", G_CALLBACK(configure_event), NULL);

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
