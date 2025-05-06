#ifndef UTILS_H
#define UTILS_H

#include "types.h"

void build_path(char *buffer, size_t buffer_size, const char *separator, int count, ...);
vec2D dvec(vec2D p0, vec2D p1);
float magnitude(vec2D vec);
vec2D normalize(vec2D vec);
float dotp(vec2D* vec1, vec2D* vec2);

#endif
