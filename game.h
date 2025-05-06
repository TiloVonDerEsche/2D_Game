#ifndef GAME_H
#define GAME_H

#include "types.h"

typedef struct {
    char* window_title;
    uint8_t game_is_running;

    float delta_time;
    int last_frame_time;

    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Texture* brick;
    SDL_Texture* dirt;
    SDL_Texture* painting;
    SDL_Texture* flag_tex;
    SDL_Texture* paladin;
    SDL_Texture* guard;
    SDL_Texture* human;

    char* food_label;
    char* colonist_label;

    float spawn_density;

    bl* grid;
    bl* colonists;
    bl* troops;
    bl* map;
    bl* collision_map;
    bl** movables[MQ];
    formation army[10];

    uint16_t* selected_troops;
    uint16_t* selected_colonists;
    uint16_t selected_troop_num;
    uint16_t selected_colonist_num;
    uint8_t formation_selector;

    uint16_t flag_limit;
    uint16_t colonist_limit;
    uint16_t troop_limit;

    uint16_t flag_width;
    uint16_t flag_height;
    uint16_t troop_width;
    uint16_t troop_height;
    uint16_t colonist_width;
    uint16_t colonist_height;
    uint16_t tile_width;
    uint16_t tile_height;
    uint16_t grid_width;
    uint16_t grid_height;

    int tiles_x;
    int tiles_y;

    int move_map_counter_x;
    int move_map_counter_y;
    bool dragging;
    bool follow_flag;

    resources res;
    vec2D mouse;
    vec2D flag;
    vec2D drag_start;
} game_state;

int initialize_window(game_state* gs);
void setup(game_state* gs);
void process_input(game_state* gs);
void update(game_state* gs);
void render(game_state* gs);
void destroy_window(game_state* gs);

#endif
