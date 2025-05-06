#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "game.h"
#include "utils.h"

size_t get_tile_index(game_state* gs, int x, int y);
void draw_grid(game_state* gs, uint16_t width, uint16_t height);
void draw_house(game_state* gs, uint16_t x, uint16_t y, uint16_t house_width, uint16_t house_height, uint16_t door_pos, SDL_Texture* tex);
void switch_color(ball* tile);
void move_map(game_state* gs, int dx, int dy);
void reset_flags(game_state* gs, uint8_t squad_num);
void set_flag(game_state* gs, vec2D flag);
void move_colonists(game_state* gs);
void move_troop(game_state* gs, uint16_t selected_troop, float troop_velo, vec2D flag);
void move_army(game_state* gs, float troop_velo);
void initialize_formations(game_state* gs);
void update(game_state* gs);

#endif
