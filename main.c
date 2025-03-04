/*
Version: 0.1.1
C Standard: C17
Author: Tilo von Eschwege
*/

#include <stdio.h>
#include <Math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "E:\res\SDL3\include\SDL3\SDL.h"
#include "physics.h"

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define FPS 144
#define FRAME_TARGET_TIME (1000/FPS)

#define TRUE 1
#define FALSE 0

#define MQ 2

typedef struct  {
  uint16_t food;
  uint16_t colonists;
} Resources;

Resources res = {0,0};

char* window_title = "2D Game";
uint8_t game_is_running = FALSE;

float delta_time = 0;
int last_frame_time = 0;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

char* food_label;
char* colonist_label;

float spawn_density = 1.0;

vec2D mouse;

bl* colonists;
bl* grid;
bl* map;

static bl* movables[MQ];

uint16_t tile_width = 50;
uint16_t tile_height = 50;

int tiles_x = 0;
int tiles_y = 0;

uint16_t grid_width = 0;
uint16_t grid_height = 0;

int move_map_counter_x = 0;
int move_map_counter_y = 0;

// uint8_t color_x_switch = 0;
// uint8_t color_y_switch = 0;


size_t get_tile_index(int x, int y) {
  int tile_x = x / tile_width;
  int tile_y = y / tile_height;

  printf("Tile: (%d, %d)\n",tile_x,tile_y);
  return tile_y * tiles_x + tile_x;
}

void draw_grid(uint16_t width, uint16_t height) {//width, height of a tile
  tiles_x = ((WINDOW_WIDTH / width) + 1);
  tiles_y = ((WINDOW_HEIGHT / height) + 1);

  grid_width = tiles_x * width;
  grid_height = tiles_y * height;

  color ca = {0, 50, 0, 50};
  color cb = {0, 0, 50, 50};
  color c;
  for (size_t y = 0; y < tiles_y; y++) {
    for (size_t x = 0; x < tiles_x; x++) {
      if ((x % 2 + y % 2) % 2) {c=ca;} else {c=cb;}
      spawn_ball(x*width, y*height, width, height, 0, c, &grid);
      grid->num++;
      printf("grid->num: %lld\n", grid->num);
    }
  }

}


void switch_color(ball* tile) {
  color ca = {0, 50, 0, 50};
  color cb = {0, 0, 50, 50};
  color c;
  c = tile->color;

  if (c.g == 50) {
    tile->color = cb;}
  else  {
    tile->color = ca;}
}

void move_map(int dx, int dy) {
  //determine largest quantity
  int maxm = movables[1]->num;
  for(size_t i = 2; i < MQ; i++) {
    if (movables[i]->num > maxm) {
      maxm = movables[i]->num;
    }
  }

  //grid excluded
  for(size_t j = 1; j < MQ; j++) {
    for(size_t i = 0; i < maxm; i++) {
      if (i < movables[j]->num) {
        movables[j]->arr[i].pos.x += dx;
        movables[j]->arr[i].pos.y += dy;
      }
    }
  }




  //grid logic
  uint8_t reset_x = 0;
  uint8_t reset_y = 0;

  //increment global counters
  move_map_counter_x += dx;
  move_map_counter_y += dy;

  //printf("First tile: (%d, %d)\n",movables[0]->arr[0].pos.x, movables[0]->arr[0].pos.y);
  printf("Counter: (%d, %d)\n",move_map_counter_x, move_map_counter_y);
  printf("Condition: %d\n",move_map_counter_y <= -tile_height);

  for(size_t i = 0; i < movables[0]->num; i++) {
      movables[0]->arr[i].pos.x += dx;
      movables[0]->arr[i].pos.y += dy;



      //modulo tile reset
      if (move_map_counter_x >= tile_width) {
        movables[0]->arr[i].pos.x -= tile_width;
        switch_color(&movables[0]->arr[i]);
        reset_x = 1;}
      else if (move_map_counter_x <= -tile_width) {
        movables[0]->arr[i].pos.x += tile_width;
        switch_color(&movables[0]->arr[i]);
        reset_x = 1;}

      if (move_map_counter_y >= tile_height) {
        movables[0]->arr[i].pos.y -= tile_height;
        switch_color(&movables[0]->arr[i]);
        reset_y = 1;}
      if (move_map_counter_y <= -tile_height) {
        movables[0]->arr[i].pos.y += tile_height;
        switch_color(&movables[0]->arr[i]);
        reset_y = 1;
      }
  }



  if (reset_x) {move_map_counter_x = 0; reset_x = 0;}
  if (reset_y) {move_map_counter_y = 0; reset_y = 0;}

}

/*
* Write a function that loops the tiles back around,
* By changing x or y if the edge of the grid is nearing the screen
*
*/




/**********************************
* Creates the window and renderer,
* While handling potential errors.
**********************************/
int initialize_window()
{
    int initErrC = SDL_Init(SDL_INIT_VIDEO);
    if (initErrC != 1)
    {
        fprintf(stderr, "An error occured, while initializing SDL.\nError Code: %d\nSDL Error:%s\n",initErrC,SDL_GetError());

        return FALSE;
    }

    window = SDL_CreateWindow(
        window_title, //window title
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_FULLSCREEN
    );

    if (!window)
    {
        fprintf(stderr, "An error occured, while creating the window.\nSDL Error:%s\n",SDL_GetError());
        return FALSE;
    }


    // int numRenders = SDL_GetNumRenderDrivers();
    // printf("NumRenderDrivers:%d\n",
    // numRenders);
    //
    // puts("\nAvailable Renders:\n");
    // for(int i = 0; i < numRenders; i++) {
    //   printf("%s\n",SDL_GetRenderDriver(i));
    // }

    renderer = SDL_CreateRenderer(window,"gpu");//driver code, display number; -1 -> default

    if (!renderer)
    {
        fprintf(stderr, "An error occured, while creating the SDL-Renderer.\nSDL Error:%s\n",SDL_GetError());
        return FALSE;
    }

    return TRUE;
}


/*********************
* Handles user-input.
*********************/
void process_input()
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type)
    {
        case SDL_EVENT_QUIT: //window's x button is clicked
            game_is_running = FALSE;
            break;

        case SDL_EVENT_KEY_DOWN: //keypress

            switch(event.key.key)
            {
              case SDLK_ESCAPE:
                game_is_running = FALSE;
                break;

              case SDLK_O:
                grid->num = 0;
                draw_grid(tile_width,tile_height);
                break;

              case SDLK_W:
                move_map(0, -10);
                break;

              case SDLK_S:
                move_map(0, 10);
                break;

              case SDLK_A:
                move_map(-10, 0);
                break;

              case SDLK_D:
                move_map(10, 0);
                break;

            }
            break;
            //printf("%d\n",event.key.key);

        case SDL_EVENT_MOUSE_MOTION:
            mouse.x = (uint16_t)event.motion.x;
            mouse.y = (uint16_t)event.motion.y;

            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            printf("mouse=(%d, %d)\n",(int)mouse.x, (int)mouse.y);
            printf("Mouse button:%d\n", event.button.button);

            switch(event.button.button) {
              case 1:
                printf("Coloring tile at index: %lld\n", get_tile_index(mouse.x,mouse.y));
                if (((int)(mouse.x/grid_width) % 2 + (int)(mouse.y/grid_height) % 2) % 2) {
                  grid->arr[get_tile_index(mouse.x,mouse.y)].color.g = 70;}
                else {
                  grid->arr[get_tile_index(mouse.x,mouse.y)].color.b = 70;}


                break;
              case 2:

                break;
              case 3:

                break;
            }
            break;

    }
}


/************************
* Gets called only once.
*************************/
void setup()
{

  grid = malloc(sizeof(bl) + 5 * sizeof(ball));
  grid->num = 0;
  grid->len = 5;

  draw_grid(tile_width,tile_height);
  printf("Last tile: (%d, %d)\n", grid->arr[grid->num - 1].pos.x, grid->arr[grid->num - 1].pos.y);
  printf("Setup function: grid->num: %lld\n", grid->num);


  colonists = malloc(sizeof(bl) + 10 * sizeof(ball));  // 20 balls
  colonists->num = 3;
  colonists->len = 10;

  map = malloc(sizeof(bl) + 5 * sizeof(ball));
  map->num = 2;
  map->len = 5;

  spawn_ball(100,100,400,50, 0, neutron_color, &colonists);
  spawn_ball(500,100,400,50, 0, electron_color, &colonists);

  movables[0] = grid;
  movables[1] = colonists;
  movables[2] = map;

  spawn_ball(500,500, 10, 10, 0, proton_color, &colonists);
  spawn_ball(520,500, 10, 10, 0, proton_color, &colonists);
  spawn_ball(540,500, 10, 10, 0, proton_color, &colonists);

  food_label = calloc(6 + sizeof(int),1); //"Food: %d"-> 6 + sizeof(int)
  colonist_label = calloc(11 + sizeof(float),1); //"Colonists: %d" -> 11+ sizeof(int)
}


/************************
* Applies physics
* To the game objects.
*************************/
void update()
{
    //delay, so that the capped framerate is reached (and not overshoot).
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
    {
        SDL_Delay(time_to_wait);
    }


    delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;

    last_frame_time = SDL_GetTicks();


}


void render_balls(bl** balls) {
  ball* b;
  for (size_t i = 0; i < (*balls)->num; i++) {

      //printf("rendering ball at (%f,%f)\n",balls[i].pos.x,balls[i].pos.y);
      b = &(*balls)->arr[i];

      if (b->pos.x >= 0 && b->pos.y >= 0) {
        SDL_FRect ball_rect =
        {
            b->pos.x,
            b->pos.y,
            b->width,
            b->height
        };

        SDL_SetRenderDrawColor(renderer, b->color.r, b->color.g, b->color.b, b->color.alpha);
        SDL_RenderFillRect(renderer, &ball_rect);
      }
  }
}

/*******************************
* Re-/Draws the window,
* With all of the game-objects.
*******************************/
void render()
{
    //background
    SDL_SetRenderDrawColor(renderer, 0, 0, 100, 255); //R, G, B, Alpha
    SDL_RenderClear(renderer);

    render_balls(&grid);
    render_balls(&colonists);
    render_balls(&map);

    sprintf(food_label, "Food: %d", res.food);
    sprintf(colonist_label, "Colonists: %d", res.colonists);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderDebugText(renderer, 14, 65, food_label);
    SDL_RenderDebugText(renderer, 14, 85, colonist_label);

    SDL_RenderPresent(renderer);

}






/***************************
* Quits the renderer,
* SDL and closes the window.
***************************/
void destroy_window()
{
    free(colonist_label);
    free(food_label);

    free(grid);
    free(colonists);
    free(map);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


/*********************************
* Initializes the window,
* Calls the setup function
* And starts the game-loop,
* With the process_input(),
* update() and render() function.
* When the game-loop terminates,
* The window gets destroyed.
*
* @param argc, @param args
*********************************/
void game() {
  printf("Game is running...\n");


  game_is_running = initialize_window();
  setup();
  while (game_is_running) //Game Loop
  {
      process_input();
      update();
      render();

  }

  destroy_window();
}

int main(int argc, char* args[])
{
    game();
    return 0;
}

/*
* The click of the mouse should spawn a ball at that position.
* That way the user doesn't need to input the ball_num
*/
