/*
Version: 0.1.5
C Standard: C17
Author: Tilo von Eschwege
*/

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "/usr/include/SDL3/SDL.h"
#include "/usr/include/SDL3_image/SDL_image.h"

#include "helper.h"

#define FPS 144
#define FRAME_TARGET_TIME (1000/FPS)

#define TRUE 1
#define FALSE 0

#define MQ 4

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define TROOP_WIDTH 100
#define TROOP_HEIGHT 100

#define COLONIST_LIMIT 100
#define TROOP_LIMIT 100




typedef struct Resources{
  uint16_t food;
  uint16_t colonists;
} resources;

typedef struct Formation {
  vec2D flags[20];
  uint16_t num;
  uint16_t troops[TROOP_LIMIT];
} formation;

resources res = {0,0};

char* window_title = "2D Game";
uint8_t game_is_running = FALSE;

float delta_time = 0;
int last_frame_time = 0;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Texture* brick = NULL;
SDL_Texture* dirt = NULL;
SDL_Texture* painting = NULL;

SDL_Texture* paladin = NULL;
SDL_Texture* guard = NULL;
SDL_Texture* human = NULL;

char* food_label;
char* colonist_label;

float spawn_density = 1.0;

vec2D mouse;
vec2D flag;

vec2D drag_start;
bool dragging = 0; //tracks holding LMB
bool follow_flag = 0;

bl* grid;
bl* colonists;
bl* troops;
bl* map;

uint16_t* selected_troops;
uint16_t* selected_colonists;

uint16_t selected_troop_num = 0;
uint16_t selected_colonist_num = 0;


bl** movables[MQ];
formation army[10];

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

  //printf("Tile: (%d, %d)\n",tile_x,tile_y);
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
      spawn_ball(x*width, y*height, width, height, c, &grid);

      //printf("grid->num: %lld\n", grid->num);
    }
  }

}

void draw_house(uint16_t x, uint16_t y, uint16_t house_width, uint16_t house_height, SDL_Texture* texture) {//width, height of a tile
  
    
  for (int i = 0; i < 4; i++) {
    if (i < 2) {
        for (int w = 0; w < house_width; w++) {
            printf("Spawning house tile at (%d,%d)\n", w*tile_width, y*tile_height + i*house_height*tile_height);
    
                        
            if (i == -1) {spawn_ball(w*tile_width, y*tile_height + i*house_height*tile_height, tile_width, tile_height, red, &map);}
            if (i == -1) {spawn_ball(w*tile_width, y*tile_height + i*house_height*tile_height, tile_width, tile_height, blue, &map);}
        }
    }
    if (i > 1) {
        for (int h = 0; i < house_height; h++) {
            printf("Spawning house tile at (%d,%d)\n", x*tile_width+ i*house_width*tile_width, h*tile_height);

            if (i == 2) {spawn_ball(x*tile_width+ i*house_width*tile_width, h*tile_height, tile_width, tile_height, yellow, &map);}
            if (i == 3) {spawn_ball(x*tile_width+ i*house_width*tile_width, h*tile_height, tile_width, tile_height, green, &map);}
        }    
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
  size_t maxm = (*movables[1])->num;

  for(size_t i = 2; i < MQ; i++) {
    if ((*movables[i])->num > maxm) {
      maxm = (*movables[i])->num;
    }
  }

  //grid excluded
  for(size_t j = 1; j < MQ; j++) {
    for(size_t i = 0; i < maxm; i++) {
      if (i < (*movables[j])->num) {
        (*movables[j])->arr[i].pos.x += dx;
        (*movables[j])->arr[i].pos.y += dy;
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
  //printf("Counter: (%d, %d)\n",move_map_counter_x, move_map_counter_y);
  //printf("Condition: %d\n",move_map_counter_y <= -tile_height);
  bl* grid_bl = *movables[0];
  for(size_t i = 0; i < grid_bl->num; i++) {
      grid_bl->arr[i].pos.x += dx;
      grid_bl->arr[i].pos.y += dy;



      //modulo tile reset
      if (move_map_counter_x >= tile_width) {
        grid_bl->arr[i].pos.x -= tile_width;
        switch_color(&grid_bl->arr[i]);
        reset_x = 1;}
      else if (move_map_counter_x <= -tile_width) {
        grid_bl->arr[i].pos.x += tile_width;
        switch_color(&grid_bl->arr[i]);
        reset_x = 1;}

      if (move_map_counter_y >= tile_height) {
        grid_bl->arr[i].pos.y -= tile_height;
        switch_color(&grid_bl->arr[i]);
        reset_y = 1;}
      if (move_map_counter_y <= -tile_height) {
        grid_bl->arr[i].pos.y += tile_height;
        switch_color(&grid_bl->arr[i]);
        reset_y = 1;
      }
  }



  if (reset_x) {move_map_counter_x = 0; reset_x = 0;}
  if (reset_y) {move_map_counter_y = 0; reset_y = 0;}

}


void move_colonists(){
  return;
}

void move_troops(float troop_velo){
  if (follow_flag) {
    ball* t;
    vec2D fv;
    vec2D fvn;

    uint16_t j;
    for(size_t i = 0; i < selected_troop_num; i++) {
      j = selected_troops[i];
      t = &troops->arr[j];
      fv = dvec(t->pos,flag); //vector from t to flag
      fvn = normalize(fv);

      t->pos.x += troop_velo * fvn.x;
      t->pos.y += troop_velo * fvn.y;
    }
  }
}



void select_troops(vec2D p0, vec2D p1) {
  selected_troop_num = 0;

  ball* t;
  for(int i = 0; i < troops->num; i++) {
    t = &troops->arr[i];
    //t in selecting rect?
    if ((t->pos.x < p1.x && t->pos.x > p0.x) || (t->pos.x > p1.x && t->pos.x < p0.x)) {
      if((t->pos.y < p1.y && t->pos.y > p0.y) || (t->pos.y > p1.y && t->pos.y < p0.y)) {
        if (selected_troop_num < TROOP_LIMIT) {
          selected_troops[selected_troop_num] = i;
          selected_troop_num++;
        }
      }
    }
  }
}



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


void setup()
{
  // vec2D p0 = {1,5};
  // vec2D p1 = {2,-1};
  //
  // vec2D p2 = dvec(p0, p1);
  //
  // printf("(%d, %d)\n",p2.x, p2.y);
  flag.x = WINDOW_WIDTH/2;
  flag.y = WINDOW_HEIGHT/2 - 200;

  grid = malloc(sizeof(bl) + 5 * sizeof(ball));
  grid->num = 1;
  grid->len = 5;

  draw_grid(tile_width,tile_height);
  //printf("Last tile: (%f, %f)\n", grid->arr[grid->num - 1].pos.x, grid->arr[grid->num - 1].pos.y);
  printf("Setup function: grid->num: %lld\n", grid->num);


  colonists = malloc(sizeof(bl) + 10 * sizeof(ball));  // 20 balls
  colonists->num = 0;
  colonists->len = 10;

  spawn_ball(500,500, 10, 10, red, &colonists);
  spawn_ball(520,500, 10, 10, red, &colonists);
  spawn_ball(540,500, 10, 10, red, &colonists);

  res.colonists = 3;

  troops = malloc(sizeof(bl) + 10 * sizeof(ball));
  troops->num = 0;
  troops->len = 10;

  spawn_ball(WINDOW_WIDTH/2, WINDOW_HEIGHT/2,30,30, red, &troops);
  spawn_ball(WINDOW_WIDTH/2 + 250, WINDOW_HEIGHT/2,30,30, red, &troops);
  spawn_ball(WINDOW_WIDTH/2 + 500, WINDOW_HEIGHT/2,30,30, red, &troops);
  spawn_ball(WINDOW_WIDTH/2 + 750, WINDOW_HEIGHT/2,30,30, red, &troops);
  spawn_ball(WINDOW_WIDTH/2 + 1000, WINDOW_HEIGHT/2,30,30, red, &troops);

  selected_colonists = malloc(COLONIST_LIMIT * sizeof(uint16_t));
  selected_troops = malloc(TROOP_LIMIT * sizeof(uint16_t));

  selected_troops[0] = 0;
  selected_troops[1] = 1;

  selected_troop_num = 2;

  map = malloc(sizeof(bl) + 10 * sizeof(ball));
  map->num = 0;
  map->len = 10;

  draw_house(5, 4, 4, 2, dirt);

  spawn_ball(100,100,400,50, blue, &map);
  spawn_ball(500,100,400,50, green, &map);

  movables[0] = &grid;
  movables[1] = &colonists;
  movables[2] = &map;
  movables[3] = &troops;


  //labels
  food_label = calloc(6 + sizeof(int),1); //"Food: %d"-> 6 + sizeof(int)
  colonist_label = calloc(11 + sizeof(float),1); //"Colonists: %d" -> 11+ sizeof(int)

  //textures
  brick = IMG_LoadTexture(renderer, "./assets/tileable_brick_ground_textures/Ground_04_Nrm.png");
  dirt = IMG_LoadTexture(renderer, "./assets/tileable_dirt_textures/Dirt_03.png");
  painting = IMG_LoadTexture(renderer, "./assets/tileable_dirt_textures/painting-27.jpg");

  paladin = IMG_LoadTexture(renderer, "./assets/entity/paladin.png");
  guard = IMG_LoadTexture(renderer, "./assets/entity/vault_guard.png");
  human = IMG_LoadTexture(renderer, "./assets/entity/human.png");
}

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
                setup();
                // grid->num = 0;
                // draw_grid(tile_width,tile_height);
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
              default:
                //keys 0 - 9
                if((48 <= event.key.key) && (event.key.key <= 57)) {
                  printf("Key %d\n",event.key.key-48);
                }

            }
            break;
            //printf("%d\n",event.key.key);

        case SDL_EVENT_MOUSE_MOTION:
            mouse.x = (uint16_t)event.motion.x;
            mouse.y = (uint16_t)event.motion.y;

            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
          switch(event.button.button) {
            case 1:
              dragging = 0;

              //put all of the troops inside of the selecting rect
              //into selected_troops
              select_troops(mouse, drag_start);
              break;
          }
          break;


        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            //printf("mouse=(%d, %d)\n",(int)mouse.x, (int)mouse.y);
            //printf("Mouse button:%d\n", event.button.button);

            switch(event.button.button) {
              case 1:
                dragging = 1;
                drag_start = mouse;
                follow_flag = 0;
                break;
              case 2:
                spawn_ball(mouse.x,mouse.y, 10, 10, red, &colonists);
                res.colonists += 1;
                break;
              case 3:
                follow_flag = 1;
                flag.x = mouse.x;
                flag.y = mouse.y;

                break;
            }
            break;

    }
}


/************************
* Applies physics
* To the game objects.
*************************/
void update()
{
    //moves all colonists
    //randomly or (wandering),
    //moving to their shops (working)
    move_colonists();

    //slerping all selected troops to the set flag (with RMB)
    move_troops(1);
    //printf("drag_start=(%f,%f)\n",drag_start.x,drag_start.y);



    //delay, so that the capped framerate is reached (and not overshoot).
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
    {
        SDL_Delay(time_to_wait);
    }


    delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;

    last_frame_time = SDL_GetTicks();
}



void render_selecting_rect() {
  if (dragging) {
    SDL_SetRenderDrawColor(renderer, 226, 235, 52, 100);

    SDL_RenderLine(renderer, drag_start.x, drag_start.y, drag_start.x, mouse.y);
    SDL_RenderLine(renderer, drag_start.x, drag_start.y, mouse.x, drag_start.y);

    SDL_RenderLine(renderer, mouse.x, mouse.y, drag_start.x, mouse.y);
    SDL_RenderLine(renderer, mouse.x, mouse.y, mouse.x, drag_start.y);
  }
}

void render_painting() {
  SDL_FRect dst;

  // SDL_GetTextureSize(texture, &dst.w, &dst.h);
  dst.x = 1000;
  dst.y = 1000;
  dst.w = 2000;
  dst.h = 500;


  /* Draw the icon */
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderTexture(renderer, painting, NULL, &dst);
}

void render_flag() {
  SDL_FRect flag_rect =
  {
      flag.x,
      flag.y,
      30,
      30
  };

  SDL_SetRenderDrawColor(renderer, 235, 146, 52, 255);
  SDL_RenderFillRect(renderer, &flag_rect);
}


void render_selected_troops(SDL_Texture* texture) {
  ball* t;
  uint16_t j;
  for(uint16_t i = 0; i < selected_troop_num; i++) {
    j = selected_troops[i];
    t = &troops->arr[j];
    SDL_FRect rect =
    {
        t->pos.x,
        t->pos.y,
        t->width,
        t->height
    };

    SDL_SetRenderDrawColor(renderer, 235, 146, 52, 255);
    //SDL_RenderFillRect(renderer, &flag_rect);
    SDL_RenderTexture(renderer, texture, NULL, &rect);
  }
}

void render_texture(bl** balls, SDL_Texture* texture) {
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

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderTexture(renderer, texture, NULL, &ball_rect);
      }
  }
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



   
    render_texture(&grid, dirt);
    render_texture(&colonists, human);
    render_texture(&troops, guard);
    render_selected_troops(paladin);
    //render_flag();
    render_balls(&map);
    render_selecting_rect();
    //render_painting();

    sprintf(food_label, "Food: %d", res.food);
    sprintf(colonist_label, "Colonists: %d", res.colonists);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderDebugText(renderer, 14, 65, food_label);
    SDL_RenderDebugText(renderer, 14, 85, colonist_label);

    SDL_RenderPresent(renderer);

}

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
