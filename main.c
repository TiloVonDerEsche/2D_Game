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
#include <string.h> //memcpy

#include "E:\\res\\SDL3\\include\\SDL3\\SDL.h" //"/usr/include/SDL3/SDL.h"
#include "E:\\res\\SDL3_image\\include\\SDL3_image\\SDL_image.h" //"/usr/include/SDL3_image/SDL_image.h"

#include "helper.h"

#define FPS 144
#define FRAME_TARGET_TIME (1000/FPS)

#define TRUE 1
#define FALSE 0

#define MQ 4

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define TROOP_WIDTH 50
#define TROOP_HEIGHT 50

#define COLONIST_WIDTH 50
#define COLONIST_HEIGHT 50

#define COLONIST_LIMIT 100
#define TROOP_LIMIT 100

#define FLAG_LIMIT 20




typedef struct Resources{
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

char* separator = "\\"; // "/"

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

uint8_t formation_selector = 0;

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

      //printf("grid->num: %ld\n", grid->num);
    }
  }

}

void draw_house(uint16_t x, uint16_t y, uint16_t house_width, uint16_t house_height) {//width, height of a tile
        for (int w = 0; w < house_width + 1; w++) {
            //printf("Spawning house tile at (%d,%d)\n",x*tile_width + w*tile_width, y*tile_height + house_height*tile_height);

            spawn_ball(x*tile_width + w*tile_width, y*tile_height, tile_width, tile_height, red, &map);
            spawn_ball(x*tile_width + w*tile_width, y*tile_height + house_height*tile_height, tile_width, tile_height, black, &map);
        }


        for (int h = 0; h < house_height + 1; h++) {
            //printf("Spawning house tile at (%d,%d)\n", x*tile_width+ house_width*tile_width, h*tile_height);

            spawn_ball(x*tile_width, y*tile_width + h*tile_height, tile_width, tile_height, yellow, &map);
            spawn_ball(x*tile_width + house_width*tile_width, y*tile_width + h*tile_height, tile_width, tile_height, green, &map);
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

////////////////////////////////////////////
//--------------Troop Movement------------//
////////////////////////////////////////////
//fills selected_troops[] with the indices of the troops in the selecting rect
void select_troops(vec2D p0, vec2D p1) {
  selected_troop_num = 0;

  ball* t;
  for(int i = 0; i < troops->num; i++) {
    t = &troops->arr[i];
    //t in selecting rect?
    if ((t->pos.x < p1.x && t->pos.x > p0.x) || (t->pos.x > p1.x && t->pos.x < p0.x)) {
      if((t->pos.y < p1.y && t->pos.y > p0.y) || (t->pos.y > p1.y && t->pos.y < p0.y)) {
        selected_troops[selected_troop_num] = i;
        selected_troop_num++;
      }
    }
  }
}

void reset_flags(uint8_t squad_num) {
  vec2D origin = {0,0};
  for(uint8_t i = 0; i < FLAG_LIMIT; i++) {
    army[squad_num].flags[i] = origin;
  }
}

void set_flag(vec2D flag) {
  formation* squat = &army[formation_selector];
  printf("Setting flag (%d,%d)\n", formation_selector, squat->last_flag);

  squat->flags[squat->last_flag] = flag;
  squat->last_flag++;
  if (squat->last_flag >= FLAG_LIMIT) {
    squat->last_flag = 0;
    reset_flags(formation_selector);
  }

  //if (last_teammate at flag)
  //  remove flag

  //coherence?
  //should teammates wait at the subflag until the last m8 of the formation arrived?

}


void move_colonists(){
  return;
}

void move_troop(uint16_t selected_troop, float troop_velo, vec2D flag){
    ball* t;
    vec2D fv;
    vec2D fvn;

    t = &troops->arr[selected_troop];
    fv = dvec(t->pos,flag); //vector from t to flag
    fvn = normalize(fv);

    t->pos.x += troop_velo * fvn.x;
    t->pos.y += troop_velo * fvn.y;
}

void move_army(float troop_velo){
  formation* squat;
  vec2D flag;
  ball* t;

  for (uint8_t i = 0; i < 10; i++) {
    squat = &army[i];
    flag = squat->flags[squat->curr_flag];

    for (uint8_t j = 0; j < squat->num; j++) {
      move_troop(squat->troops[j], 1, flag);
      t = &troops->arr[squat->troops[j]];

      if (i == 0) {printf("Distance to flag: %f\n",magnitude(dvec(t->pos, flag)));}
      if (magnitude(dvec(t->pos, flag)) < 1) {
        puts("YES!\n\n\n");
        squat->curr_flag += 1;
        if (squat->curr_flag >= FLAG_LIMIT) {
          squat->curr_flag = 0;
          reset_flags(i);
        }

      }
    }
  }
}



void initialize_formations() {
  for (uint8_t i = 0; i < 10; i++) {
    army[i].num = 0;
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

  //----------------------------------------------BL initialization----------------------------------------------------//
  grid = malloc(sizeof(bl) + 5 * sizeof(ball));
  grid->num = 1;
  grid->len = 5;

  draw_grid(tile_width,tile_height);
  //printf("Last tile: (%f, %f)\n", grid->arr[grid->num - 1].pos.x, grid->arr[grid->num - 1].pos.y);
  //printf("Setup function: grid->num: %ld\n", grid->num);


  colonists = malloc(sizeof(bl) + 10 * sizeof(ball));  // 20 balls
  colonists->num = 0;
  colonists->len = 10;

  spawn_ball(500,500, COLONIST_WIDTH, COLONIST_HEIGHT, red, &colonists);
  spawn_ball(550,500, COLONIST_WIDTH, COLONIST_HEIGHT, red, &colonists);
  spawn_ball(600,500, COLONIST_WIDTH, COLONIST_HEIGHT, red, &colonists);

  res.colonists = 3;

  troops = malloc(sizeof(bl) + 10 * sizeof(ball));
  troops->num = 0;
  troops->len = 10;

  spawn_ball(WINDOW_WIDTH/2, WINDOW_HEIGHT/2, TROOP_WIDTH, TROOP_HEIGHT, red, &troops);
  spawn_ball(WINDOW_WIDTH/2 + 250, WINDOW_HEIGHT/2,TROOP_WIDTH, TROOP_HEIGHT, red, &troops);
  spawn_ball(WINDOW_WIDTH/2 + 500, WINDOW_HEIGHT/2,TROOP_WIDTH, TROOP_HEIGHT, red, &troops);
  spawn_ball(WINDOW_WIDTH/2 + 750, WINDOW_HEIGHT/2,TROOP_WIDTH, TROOP_HEIGHT, red, &troops);
  spawn_ball(WINDOW_WIDTH/2 + 1000, WINDOW_HEIGHT/2,TROOP_WIDTH, TROOP_HEIGHT, red, &troops);

  selected_colonists = malloc(COLONIST_LIMIT * sizeof(uint16_t));
  selected_troops = malloc(TROOP_LIMIT * sizeof(uint16_t));

  selected_troop_num = 0;

  initialize_formations();

  // vec2D fp = {500,500};
  //
  // for (uint8_t i = 0; i < 10; i++) {
  //   army[i].flags[0] = fp;
  //   army[i].last_flag += 1;
  //   fp.x += 100;
  // }

  map = malloc(sizeof(bl) + 10 * sizeof(ball));
  map->num = 0;
  map->len = 10;

  draw_house(5, 4, 4, 3);
  draw_house(16, 14, 8, 6);

  //fill movables with all BLs that should move after pressing WASD
  movables[0] = &grid;
  movables[1] = &colonists;
  movables[2] = &map;
  movables[3] = &troops;



  //----------------------------------------------labels----------------------------------------------------//
  food_label = calloc(6 + sizeof(int),1); //"Food: %d"-> 6 + sizeof(int)
  colonist_label = calloc(11 + sizeof(float),1); //"Colonists: %d" -> 11+ sizeof(int)



  //---------------------------------------------textures----------------------------------------------------//
  char path[512];

  build_path(path, sizeof(path), separator, 3, "assets", "tileable_brick_ground_textures", "Ground_03.png");
  brick = IMG_LoadTexture(renderer, path);

  build_path(path, sizeof(path), separator, 3, "assets", "tileable_dirt_textures", "Dirt_03.png");
  dirt = IMG_LoadTexture(renderer, path); //".\\assets\\tileable_dirt_textures\\Dirt_03.png"

  build_path(path, sizeof(path), separator, 3, "assets", "tileable_dirt_textures", "painting-27.jpg");
  painting = IMG_LoadTexture(renderer, path);

  build_path(path, sizeof(path), separator, 3, "assets", "entity", "paladin.png");
  paladin = IMG_LoadTexture(renderer, path);

  build_path(path, sizeof(path), separator, 3, "assets", "entity", "vault_guard.png");
  guard = IMG_LoadTexture(renderer, path);

  build_path(path, sizeof(path), separator, 3, "assets", "entity", "human.png");
  human = IMG_LoadTexture(renderer, path);

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

        case SDL_EVENT_KEY_DOWN:
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
                //formation setting w holding LCTRL + keys 0-9
                if (event.key.mod & SDL_KMOD_LCTRL) {
                  if((48 <= event.key.key) && (event.key.key <= 57)) {
                    //printf("LCTRL + Key %d pressed!\n",event.key.key-48);
                    if (selected_troop_num > 0) {
                      uint8_t key_n = event.key.key-48;
                      memcpy(army[key_n].troops, selected_troops, selected_troop_num * sizeof(uint16_t));
                      army[key_n].num = selected_troop_num;
                      formation_selector = key_n;
                    }
                  }
                }
                //formation selection w keys 0-9
                else if((48 <= event.key.key) && (event.key.key <= 57)) {
                  printf("Key %d!!!\n",event.key.key-48);
                  uint8_t key_n = event.key.key-48;
                  formation_selector = key_n;


                  memcpy(selected_troops, army[key_n].troops, army[key_n].num * sizeof(uint16_t));
                  selected_troop_num = army[key_n].num;
                  follow_flag = 0;
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
                spawn_ball(mouse.x,mouse.y, COLONIST_WIDTH, COLONIST_HEIGHT, red, &colonists);
                res.colonists += 1;
                break;
              case 3:
                puts("RMB! set_flag trig\n");
                set_flag(mouse);
                follow_flag = 1;
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
    move_army(1);
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

void render_flags() {
  vec2D flag;

  for (uint8_t i = 0; i < 10; i++) {
    //printf("j = %d; j < %d\n" ,army[i].curr_flag ,army[i].last_flag);
    for (uint8_t j = army[i].curr_flag; j < army[i].last_flag; j++) {
      //printf("Rendering flag: (%d,%d)",i,j);
      flag = army[i].flags[j];

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
  }

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




    //render_texture(&grid, dirt);
    render_balls(&grid);
    render_texture(&colonists, human);
    render_texture(&troops, guard);
    render_selected_troops(paladin);
    render_flags();
    //render_balls(&map);
    render_texture(&map, brick);
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
    free(troops);
    free(selected_colonists);
    free(selected_troops);
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
