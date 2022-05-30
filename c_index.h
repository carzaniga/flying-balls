#ifndef C_INDEX_H_INCLUDED
#define C_INDEX_H_INCLUDED

#include "balls.h"

void c_index_build();
void c_index_check_collisions(void (*collision)(ball *, ball *));
int c_index_init();
void c_index_destroy();

#endif
