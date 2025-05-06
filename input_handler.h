#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "game.h"
#include "game_logic.h"

void process_input(game_state* gs);
void select_troops(game_state* gs, vec2D p0, vec2D p1);

#endif
