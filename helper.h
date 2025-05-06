/*
Version: 1.0.0
C Standard: C17
Author: Tilo von Eschwege
*/

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "/usr/include/SDL3/SDL.h" //"E:\\res\\SDL3\\include\\SDL3\\SDL.h"



typedef struct Vec2D {
  float x;
  float y;
} vec2D;

typedef struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t alpha; //transparency
} color;

typedef struct Ball
{
    vec2D pos;

    uint16_t width;
    uint16_t height;

    color color;
    SDL_Texture* texture;
} ball;

typedef struct BL {
  size_t len; //length of arr
  size_t num; //number of balls
  ball arr[];
} bl;



////////////////////////////////////////////////////////////////
//----------------------global variables----------------------//
////////////////////////////////////////////////////////////////
color no_color = {0,0,0,0};
color black = {0, 0, 0, 255};
color red = {255, 0, 0, 255};
color green = {0, 255, 0, 100};
color blue = {0, 0, 255, 255};
color yellow = {255, 255, 0, 255};
color white = {255, 255, 255, 255};

////////////////////////////////////////////////////////////////
//-------------------------Math_Functions---------------------//
////////////////////////////////////////////////////////////////
vec2D dvec(vec2D p0, vec2D p1) {
  return (vec2D) {p1.x - p0.x, p1.y - p0.y};
}

float magnitude(vec2D vec) {
  return sqrt(pow(vec.x,2) + pow(vec.y,2));
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
  return vec1->x*vec2->x + vec1->y*vec2->y;
}
////////////////////////////////////////////////////////////////
//-----------------------Helper_Functions---------------------//
////////////////////////////////////////////////////////////////
void build_path(char *buffer, size_t buffer_size, const char *separator, int count, ...) {
    va_list args;
    va_start(args, count);

    // Initialer Punkt
    snprintf(buffer, buffer_size, ".");

    // Füge jedes Element hinzu
    for (int i = 0; i < count; i++) {
        const char *component = va_arg(args, const char *);
        snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), "%s%s", separator, component);
    }

    va_end(args);
}

//Usage:
// int main() {
//     char path[256];
//     const char *separator = "/";
//
//     build_path(path, sizeof(path), separator, 3, "FOLDER1", "FOLDER2", "file.txt");
//     printf("Pfad: %s\n", path); // Ausgabe: ./FOLDER1/FOLDER2/file.txt
//
//     return 0;
// }

////////////////////////////////////////////////////////////////
//-----------------------Ball_Functions-----------------------//
////////////////////////////////////////////////////////////////
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
