/*
Version: 0.0.6
C Standard: C17
Author: Tilo von Eschwege
*/

#include <stdio.h>
#include <Math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "E:\res\SDL3\include\SDL3\SDL.h"


#define SOFTENING_FACTOR 1

#define PI 3.141

//gravity
#define G 10 //actually 6.674e-11, but nothing happens when i plug the correct value in
#define MIN_ACTION_DISTANCE 1e-3

//coulomb_force
#define COULOMB_CONSTANT -1e5

#define MAGNETIC_FIELD_CONTSTANT 5//4e-7//1.256637e-6 / 4PI


typedef struct Vec2D {
  int x;
  int y;
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
    vec2D velo;
    vec2D accel;


    uint16_t width;
    uint16_t height;
    float density;
    float mass;

    float charge;
    color color;
} ball;

typedef struct BL {
  size_t len; //length of arr
  size_t num; //number of balls
  ball arr[];
} bl;



////////////////////////////////////////////////////////////////
//----------------------global variables----------------------//
////////////////////////////////////////////////////////////////
color proton_color = {255, 0, 0, 255};
color neutron_color = {0, 255, 0, 100};
color electron_color = {0, 0, 255, 255};



////////////////////////////////////////////////////////////////
//-----------------------Helper_Functions---------------------//
////////////////////////////////////////////////////////////////
float magnitude(vec2D* vec) {
  return sqrt(pow(vec->x,2) + pow(vec->y,2));
}

vec2D normalize(vec2D* vec, float mag) {
  vec2D vec_norm;

  if (mag > 0) {
    vec_norm.x = vec->x / mag;
    vec_norm.y = vec->y / mag;
  } else {
    vec_norm.x = 0;
    vec_norm.y = 0;
  }

  return vec_norm;
}

float dotp(vec2D* vec1, vec2D* vec2) {
  return vec1->x*vec2->x + vec1->y*vec2->y;
}

float get_gravity(ball* b1, ball* b2, float d) {
  return G * (b1->mass * b2->mass) / (pow(d,2) + pow(SOFTENING_FACTOR,2));
}

float get_coulomb(ball* b1, ball* b2, float d) {
  return COULOMB_CONSTANT * b1->charge * b2->charge / (pow(d,2) + pow(SOFTENING_FACTOR,2));
}

float magnetic_field(ball* b, float velo ,float theta, float d) {
  return MAGNETIC_FIELD_CONTSTANT * b->charge * velo * sin(theta) / (pow(d,2) + pow(SOFTENING_FACTOR,2));
}


////////////////////////////////////////////////////////////////
//-----------------------Ball_Functions-----------------------//
////////////////////////////////////////////////////////////////
void init_ball(ball* b, uint16_t width, uint16_t height, float density) {
  b->velo = (vec2D){0, 0};
  b->accel = (vec2D){0, 0};

  b->width = width;
  b->height = height;

  b->density = density;
  b->mass = b->density * ((float)(b->width * b->height));
}

void spawn_ball(int x, int y, uint16_t width, uint16_t height, float charge, color color, bl** balls) {
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
  b->charge = charge;
  b->color = color;
  //printf("Spawning Ball at (%d, %d), with index: %d\n",x, y, ball_num);

  init_ball(b, width, height, 1);

  (*balls)->num++;
}

/******************************************
* Applies the x and y speed
* to the x and y coordiantes of the balls
* Handles collision of the balls
* and the window border.
******************************************/


void update_ball_movement(float dt, bl* balls)
{
    ball* b;
    for (int i = 0; i < balls->num; i++)
    {
        b = &balls->arr[i];

        b->velo.x += b->accel.x * dt;
        b->velo.y += b->accel.y * dt;

        b->pos.x += b->velo.x * dt;
        b->pos.y += b->velo.y * dt;

    }
}


void apply_forces(bl* balls)
{
    float gravity;
    float coulomb_force;
    float magnetic_force;

    vec2D net_force;

    vec2D vec;
    vec2D vec_norm;
    float d;

    vec2D velo;
    vec2D velo_norm;
    float mag_velo;

    float theta;

    ball* b1;
    ball* b2;
    for (int i = 0; i < balls->num; i++)
    {
        b1 = &balls->arr[i];
        net_force.x = 0.0;
        net_force.y = 0.0;

        for (int j = 0; j < balls->num; j++)
        {
            if (i != j) {
              b2 = &balls->arr[j];
              vec.x = b2->pos.x - b1->pos.x;
              vec.y = b2->pos.y - b1->pos.y;

              d = magnitude(&vec); //distance btw b1 & b2
              if (d != 0) {
                vec_norm = normalize(&vec, d);

                velo = b2->velo;
                mag_velo = magnitude(&velo);
                velo_norm = normalize(&velo, mag_velo);

                theta = acos(fmax(1, fmin(-1, dotp(&vec_norm, &velo_norm))));


                  if (d > MIN_ACTION_DISTANCE) {
                    gravity = get_gravity(b1,b2,d);
                    //the normalized vector is multiplied with the gravitational force
                    net_force.x +=  vec_norm.x * gravity;
                    net_force.y +=  vec_norm.y * gravity;


                    //Lorentz Force
                    coulomb_force = get_coulomb(b1,b2,d);
                    net_force.x += vec_norm.x * coulomb_force;
                    net_force.y += vec_norm.y * coulomb_force;

                    magnetic_force = magnetic_field(b2, mag_velo, theta, d);
                    net_force.x += vec_norm.x * magnetic_force;
                    net_force.y += vec_norm.y * magnetic_force;
                  }
              }

            }


        }

        if (b1->mass != 0) {
          b1->accel.x = net_force.x / b1->mass; // F = m * a -> a = F / m
          b1->accel.y = net_force.y / b1->mass;
        }


    }
}
