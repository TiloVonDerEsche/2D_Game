#include "input_handler.h"
#include <string.h>

void select_troops(game_state* gs, vec2D p0, vec2D p1) {
  gs->selected_troop_num = 0;
  ball* t;
  for (int i = 0; i < gs->troops->num; i++) {
    t = &gs->troops->arr[i];
    if ((t->pos.x < p1.x && t->pos.x > p0.x) || (t->pos.x > p1.x && t->pos.x < p0.x))
    {
      if ((t->pos.y < p1.y && t->pos.y > p0.y) || (t->pos.y > p1.y && t->pos.y < p0.y))
      {
        gs->selected_troops[gs->selected_troop_num] = i; gs->selected_troop_num++;
      }
    }
  }
}



void process_input(game_state* gs) {
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type) {
        case SDL_EVENT_QUIT:
            gs->game_is_running = FALSE;
            break;
        case SDL_EVENT_KEY_DOWN:
            printf("Key %d!!!\n",event.key.key);
            switch (event.key.key) {
                case SDLK_ESCAPE:
                    gs->game_is_running = FALSE;
                    break;
                case SDLK_PLUS:
                    puts("PLUS!!");
                    gs->flag_width += 1;
                    gs->flag_height += 1;
                    gs->troop_width += 1;
                    gs->troop_height += 1;
                    gs->colonist_width += 1;
                    gs->colonist_height += 1;
                    gs->tile_width += 1;
                    gs->tile_height += 1;
                    setup(gs);
                    break;
                case SDLK_MINUS:
                    gs->flag_width -= 1;
                    gs->flag_height -= 1;
                    gs->troop_width -= 1;
                    gs->troop_height -= 1;
                    gs->colonist_width -= 1;
                    gs->colonist_height -= 1;
                    gs->tile_width -= 1;
                    gs->tile_height -= 1;
                    setup(gs);
                    break;
                case SDLK_O:
                    setup(gs);
                    break;
                case SDLK_W:
                    move_map(gs, 0, -10);
                    break;
                case SDLK_S:
                    move_map(gs, 0, 10);
                    break;
                case SDLK_A:
                    move_map(gs, -10, 0);
                    break;
                case SDLK_D:
                    move_map(gs, 10, 0);
                    break;
                default:
                  //formation setting w holding LCTRL + keys 0-9
                  if (event.key.mod & SDL_KMOD_LCTRL) {
                    if((48 <= event.key.key) && (event.key.key <= 57)) {
                      //printf("LCTRL + Key %d pressed!\n",event.key.key-48);
                      if (gs->selected_troop_num > 0) {
                        uint8_t key_n = event.key.key-48;
                        memcpy(gs->army[key_n].troops, gs->selected_troops, gs->selected_troop_num * sizeof(uint16_t));
                        gs->army[key_n].num = gs->selected_troop_num;
                        gs->formation_selector = key_n;
                      }
                    }
                  }
                  //formation selection w keys 0-9
                  else if((48 <= event.key.key) && (event.key.key <= 57)) {
                    printf("Key %d!!!\n",event.key.key-48);
                    uint8_t key_n = event.key.key-48;
                    gs->formation_selector = key_n;


                    memcpy(gs->selected_troops, gs->army[key_n].troops, gs->army[key_n].num * sizeof(uint16_t));
                    gs->selected_troop_num = gs->army[key_n].num;
                    gs->follow_flag = 0;
                  }
            }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            gs->mouse.x = (uint16_t)event.motion.x;
            gs->mouse.y = (uint16_t)event.motion.y;
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
          switch(event.button.button) {
            case 1:
              gs->dragging = 0;

              //put all of the troops inside of the selecting rect
              //into selected_troops
              select_troops(gs, gs->mouse, gs->drag_start);
              break;
          }
          break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            switch (event.button.button) {
                case 1:
                  gs->dragging = 1;
                  gs->drag_start = gs->mouse;
                  gs->follow_flag = 0;
                  break;
                case 2:
                    spawn_ball(gs->mouse.x, gs->mouse.y, gs->colonist_width, gs->colonist_height, &gs->colonists, red, gs->human);
                    gs->res.colonists += 1;
                    break;
                case 3:
                  //puts("RMB! set_flag trig\n");
                  set_flag(gs, gs->mouse);
                  gs->follow_flag = 1;
                  break;
            }
            break;
    }
}
