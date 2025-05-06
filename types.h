#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "E:\\res\\SDL3\\include\\SDL3\\SDL.h"
#include "E:\\res\\SDL3_image\\include\\SDL3_image\\SDL_image.h"
#include "constants.h"

typedef struct Vec2D {
    float x;
    float y;
} vec2D;

typedef struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t alpha;
} color;

typedef struct Ball {
    vec2D pos;
    uint16_t width;
    uint16_t height;
    color color;
    SDL_Texture* texture;
} ball;

typedef struct BL {
    size_t len;
    size_t num;
    ball arr[];
} bl;

typedef struct Resources {
    uint16_t food;
    uint16_t colonists;
} resources;

typedef struct Formation {
    uint16_t curr_flag;
    uint16_t last_flag;
    vec2D flags[FLAG_LIMIT];
    uint16_t num;
    uint16_t troops[TROOP_LIMIT];
} formation;

extern color no_color;
extern color black;
extern color red;
extern color green;
extern color blue;
extern color yellow;
extern color white;

void init_ball(ball* b, uint16_t width, uint16_t height);
void spawn_ball(int x, int y, uint16_t width, uint16_t height, bl** balls, color color, SDL_Texture* texture);

#endif
