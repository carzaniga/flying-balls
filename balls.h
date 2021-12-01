#ifndef BALLS_H_INCLUDED
#define BALLS_H_INCLUDED

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

extern unsigned int width;
extern unsigned int height;

extern struct ball * balls;
extern unsigned int n_balls;


#endif
