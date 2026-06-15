#ifndef RENDERER_H
#define RENDERER_H

#include "game.h"

void render(game_state* gs);
void render_selecting_rect(game_state* gs);
void render_flags(game_state* gs, SDL_Texture* texture);
void render_selected_troops(game_state* gs, SDL_Texture* texture);
void render_ball(game_state* gs, ball* b);
void render_balls(game_state* gs, bl** balls);

#endif
