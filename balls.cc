#include <vector>
#include <cmath>
#include <cassert>

#include "game.h"
#include "balls.h"
#include "gravity.h"

unsigned int radius_min = 5;
unsigned int radius_max = 10;

unsigned int v_max = 100;
unsigned int v_min = 0;

unsigned int v_angle_min = 0;
unsigned int v_angle_max = 100;

ball * balls = nullptr;
unsigned int n_balls = 50;

// Coefficient of restitution:
// C_r == 1.0 ==> perfectly elastic collisions
// C_r == 0.0 ==> perfectly inelastic collisions
//
static double C_r = 1.0;

void restitution_coefficient_set (double c) {
    C_r = c;
    if (C_r > 1.0)
	C_r = 1.0;
    else if (C_r < 0.0)
	C_r = 0.0;
}

double restitution_coefficient_get () {
    return C_r;
}

void restitution_coefficient_change (double d) {
    C_r += d;
    if (C_r > 1.0)
	C_r = 1.0;
    else if (C_r < 0.0)
	C_r = 0.0;
}

void restitution_coefficient_draw (cairo_t * cr) {
    static const double margin = 20;
    cairo_save(cr);
    cairo_new_path(cr);
    cairo_move_to(cr, margin, margin);
    cairo_line_to(cr, margin + (width - 2*margin)*C_r, margin);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_set_line_width(cr, margin/2);
    cairo_stroke(cr);
    cairo_restore(cr);
}

static vec2d random_velocity() {
    double r2;
    vec2d v;
    do {
	v.x = v_min + rand() % (v_max + 1 - v_min);
	v.y = v_min + rand() % (v_max + 1 - v_min);
	r2 = vec2d::dot(v,v);
    } while (r2 > v_max*v_max || r2 < v_min*v_min);
    if (rand() % 2)
	v.x = -v.x;
    if (rand() % 2)
	    v.y = -v.y;
    return v;
}

void balls_init_state () {
    static const int border = 10;
    int w = width < 2*border ? 1 : width - 2*border;
    int h = height < 2*border ? 1 : height - 2*border;

    for (unsigned int i = 0; i < n_balls; ++i) {
	balls[i].position.x = border + rand() % w;
	balls[i].position.y = border + rand() % h;
	balls[i].velocity = random_velocity();
	balls[i].radius = radius_min + rand() % (radius_max + 1 - radius_min);
	unsigned int v_angle_360 = (v_angle_min + rand() % (v_angle_max + 1 - v_angle_min)) % 360;
	balls[i].v_angle = 2*M_PI*v_angle_360/360;
	balls[i].angle = (rand() % 360)*2*M_PI/360;
    }
}

void ball_walls_collision (ball * p) {
    if (p->position.x + p->radius > width) { /* right wall */
	if (p->velocity.x > 0) {
	    p->position.x -= p->position.x + p->radius - width;
	    p->velocity.x = -C_r*p->velocity.x;
	}
    } else if (p->position.x < p->radius) { /* left wall */
	if (p->velocity.x < 0) {
	    p->position.x += p->radius - p->position.x;
	    p->velocity.x = -C_r*p->velocity.x;
	}
    } 
    if (p->position.y + p->radius > height) { /* bottom wall */
	if (p->velocity.y > 0) {
	    p->position.y -= p->position.y + p->radius - height;
	    p->velocity.y = -C_r*p->velocity.y;
	}
    } else if (p->position.y < p->radius) { /* top wall */
	if (p->velocity.y < 0) {
	    p->position.y += p->radius - p->position.y;
	    p->velocity.y = -C_r*p->velocity.y;
	}
    }
}

void ball_update_state (ball * p) {
    vec2d g = gravity_vector (p);

    p->position += delta*p->velocity + delta*delta*g/2.0;
    p->velocity += delta*g;
    p->angle += delta*p->v_angle;
    while (p->angle >= 2*M_PI)
	p->angle -= 2*M_PI;
    while (p->angle < 0)
	p->angle += 2*M_PI;
    ball_walls_collision (p);
}

void ball_ball_collision (ball * p, ball * q) {
    vec2d pq = q->position - p->position;
    double d2 = vec2d::dot(pq,pq);
    double r = p->radius + q->radius;
    if (d2 <= r*r) {
	vec2d pq_v = q->velocity - p->velocity;

	double mp = p->radius * p->radius;
	double mq = q->radius * q->radius;
	double m_total = mp + mp;

	double d = sqrt(d2);
	vec2d pq_overlap = (r - d)/d*pq;
	p->position -= pq_overlap*mq/m_total;
	q->position += pq_overlap*mp/m_total;

	double f = vec2d::dot(pq_v, pq);

	if (f < 0) {
	    f /= d2*(mp + mq);
	    p->velocity += 2*C_r*mq*f*pq;
	    q->velocity -= 2*C_r*mp*f*pq;
	}
    }
}

void ball_reposition (ball * b) {
    if (b->position.x < b->radius)
	b->position.x = b->radius;
    else if (b->position.x + b->radius > width)
	b->position.x = width - b->radius;
    if (b->position.y < b->radius)
	b->position.y = b->radius;
    else if (b->position.y + b->radius > height)
	b->position.y = height - b->radius;
}

const char * face_filename = 0;
int face_rotation = 0;

static const double linear_rotation_unit = 2.0;

static std::vector<ball_face *> faces;

class ball_face {
public:
    ball_face (unsigned int radius, cairo_surface_t * face, int rotation);
    ~ball_face ();
    cairo_surface_t * get_surface (double angle) const;
private:
    unsigned int rotations;
    std::vector <cairo_surface_t *> c_faces;
};

static double random_color_component() {
    return 1.0*(rand() % 200 + 56)/255;
};

ball_face::ball_face (unsigned int radius, cairo_surface_t * face, int rotation) {
    if (face && rotation) {
	rotations = 2*M_PI * radius / linear_rotation_unit;
    } else {
	rotations = 1;
    }
    c_faces.resize(rotations);
    for (unsigned int i = 0; i < rotations; ++i) {
	c_faces[i] = gdk_window_create_similar_surface(gtk_widget_get_window(canvas),
						       CAIRO_CONTENT_COLOR_ALPHA,
						       2*radius, 2*radius);
	assert(c_faces[i]);
	cairo_t * ball_cr = cairo_create(c_faces[i]);
	cairo_translate(ball_cr, radius, radius);
	cairo_arc(ball_cr, 0.0, 0.0, radius, 0, 2 * M_PI);
	cairo_clip(ball_cr);

	if (face) {
	    int face_x_offset = cairo_image_surface_get_width (face) / 2;
	    int face_y_offset = cairo_image_surface_get_height (face) / 2;
	    cairo_rotate(ball_cr, i*2*M_PI/rotations);
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
	    cairo_pattern_destroy (pat);
	}
	cairo_surface_flush(c_faces[i]);
	cairo_destroy(ball_cr);
    }
}

ball_face::~ball_face() {
    for (auto f : c_faces)
	cairo_surface_destroy(f);
}

cairo_surface_t * ball_face::get_surface (double angle) const {
    unsigned int face_id;
    if (rotations == 1)
	face_id = 0;
    else {
	face_id = rotations*angle/(2*M_PI);
	assert(face_id < rotations);
	if (face_id >= rotations)
	    face_id %= rotations;
    }
    return c_faces[face_id];
}

static void balls_init_faces () {
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
	faces.assign(radius_max + 1 - radius_min, nullptr);
	for(ball * b = balls; b != balls + n_balls; ++b) {
	    unsigned int r_idx = b->radius - radius_min;
	    if (faces[r_idx] == nullptr)
		faces[r_idx] = new ball_face (b->radius, face_surface, face_rotation);
	    b->face = faces[r_idx];
	}
	cairo_surface_destroy (face_surface);
    } else {
	faces.resize(n_balls);
	for (unsigned int i = 0; i < n_balls; ++i)
	    balls[i].face = faces[i] = new ball_face (balls[i].radius, 0, face_rotation);
    }
}

void ball::draw (cairo_t * cr) const {
    cairo_save (cr);
    cairo_translate (cr, position.x - radius, position.y - radius);
    cairo_set_source_surface(cr, face->get_surface (angle), 0, 0);
    cairo_paint(cr);
    cairo_restore(cr);
}

void balls_draw (cairo_t * cr) {
    for (const ball * b = balls; b != balls + n_balls; ++b)
	b->draw(cr);
}

static void balls_destroy_faces () {
    for (ball_face * f : faces)
	if (f)
	    delete (f);
    faces.clear();
}

void balls_destroy () {
    balls_destroy_faces ();
    delete [] (balls);
}

void balls_init () {
    balls = new ball[n_balls];
    assert(balls);
    balls_init_state ();
    balls_init_faces ();
}
