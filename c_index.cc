#include <stdlib.h>

#include "balls.h"
#include "game.h"
#include "c_index.h"

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
    for(unsigned int i = 1; i < n_balls; ++i)
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

void c_index_check_collisions(void (*collision)(struct ball *, struct ball *)) {
    for(struct ball * b = balls + 1; b < balls + n_balls; ++b) {
	c_index_stack_clear();
	struct bt_node * n = c_index;
	do {
	    (*collision)(n->ball, b);
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
	c_index = (struct bt_node *) malloc(sizeof(struct bt_node) * n_balls);
    if (!c_index)
	return 0;
    if (!c_index_stack)
	c_index_stack = (struct bt_node **) malloc(sizeof(struct bt_node *) * n_balls);
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

