#include "utils.h"
#include <stdarg.h>

void build_path(char *buffer, size_t buffer_size, const char *separator, int count, ...) {
    va_list args;
    va_start(args, count);
    snprintf(buffer, buffer_size, ".");
    for (int i = 0; i < count; i++) {
        const char *component = va_arg(args, const char *);
        snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), "%s%s", separator, component);
    }
    va_end(args);
}

vec2D dvec(vec2D p0, vec2D p1) {
    return (vec2D){p1.x - p0.x, p1.y - p0.y};
}

float magnitude(vec2D vec) {
    return sqrt(pow(vec.x, 2) + pow(vec.y, 2));
}

vec2D normalize(vec2D vec) {
    vec2D vec_norm;
    float mag = magnitude(vec);
    if (mag > 0) {
        vec_norm.x = vec.x / mag;
        vec_norm.y = vec.y / mag;
    } else {
        vec_norm.x = 0;
        vec_norm.y = 0;
    }
    return vec_norm;
}

float dotp(vec2D* vec1, vec2D* vec2) {
    return vec1->x * vec2->x + vec1->y * vec2->y;
}


void init_ball(ball* b, uint16_t width, uint16_t height) {
  b->width = width;
  b->height = height;
}

void spawn_ball(int x, int y, uint16_t width, uint16_t height, bl** balls, color color, SDL_Texture* texture) {
  if ((*balls)->len <= (*balls)->num + 1) {
    (*balls)->len *= 2;

    printf("Reallocating balls, ball_num:%lld, new size:%lld...\n",(*balls)->num, (*balls)->len);
    *balls = realloc(*balls, sizeof(bl) + ((*balls)->len) * sizeof(ball));  // Increase size by 10
    if (!*balls) {
      fprintf(stderr, "Realloc of balls failed!\nball_num=%lld, len_balls_arr=%lld"
        ,(*balls)->num, (*balls)->len);
      exit(-1);
    }
    printf("Reallocated balls! New Size:%lld\n",(*balls)->len);
  }

  ball* b = &(*balls)->arr[(*balls)->num];

  b->pos = (vec2D){x, y};
  b->color = color;
  b->texture = texture;
  //printf("Spawning Ball at (%d, %d), with index: %d\n",x, y, ball_num);

  init_ball(b, width, height);

  (*balls)->num++;
}
