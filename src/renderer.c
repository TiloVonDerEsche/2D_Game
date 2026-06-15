#include "renderer.h"
#include <string.h>

void render_selecting_rect(game_state* gs) {
    if (gs->dragging) {
        SDL_SetRenderDrawColor(gs->renderer, 226, 235, 52, 100);
        SDL_RenderLine(gs->renderer, gs->drag_start.x, gs->drag_start.y, gs->drag_start.x, gs->mouse.y);
        SDL_RenderLine(gs->renderer, gs->drag_start.x, gs->drag_start.y, gs->mouse.x, gs->drag_start.y);
        SDL_RenderLine(gs->renderer, gs->mouse.x, gs->mouse.y, gs->drag_start.x, gs->mouse.y);
        SDL_RenderLine(gs->renderer, gs->mouse.x, gs->mouse.y, gs->mouse.x, gs->drag_start.y);
    }
}

void render_flags(game_state* gs, SDL_Texture* texture) {
    vec2D flag;
    for (uint8_t i = 0; i < 10; i++) {
        for (uint8_t j = gs->army[i].curr_flag; j < gs->army[i].last_flag; j++) {
            flag = gs->army[i].flags[j];
            SDL_FRect rect = {flag.x, flag.y, gs->flag_width, gs->flag_height};
            SDL_SetRenderDrawColor(gs->renderer, 235, 146, 52, 255);
            SDL_RenderTexture(gs->renderer, texture, NULL, &rect);
        }
    }
}

void render_selected_troops(game_state* gs, SDL_Texture* texture) {
    ball* t;
    uint16_t j;
    for (uint16_t i = 0; i < gs->selected_troop_num; i++) {
        j = gs->selected_troops[i];
        t = &gs->troops->arr[j];
        SDL_FRect rect = {t->pos.x, t->pos.y, t->width, t->height};
        SDL_SetRenderDrawColor(gs->renderer, 235, 146, 52, 255);
        SDL_RenderTexture(gs->renderer, texture, NULL, &rect);
    }
}

void render_ball(game_state* gs, ball* b) {
    if (b->pos.x >= 0 && b->pos.y >= 0) {
        SDL_FRect ball_rect = {b->pos.x, b->pos.y, b->width, b->height};
        if (b->texture != NULL) {
            SDL_RenderTexture(gs->renderer, b->texture, NULL, &ball_rect);
        } else if (b->color.r != 0 || b->color.g != 0 || b->color.b != 0 || b->color.alpha != 0) {
            SDL_SetRenderDrawColor(gs->renderer, b->color.r, b->color.g, b->color.b, b->color.alpha);
            SDL_RenderFillRect(gs->renderer, &ball_rect);
        }
    }
}

void render_balls(game_state* gs, bl** balls) {
    ball* b;
    for (size_t i = 0; i < (*balls)->num; i++) {
        b = &(*balls)->arr[i];
        render_ball(gs, b);
    }
}

void render(game_state* gs) {
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 100, 255);
    SDL_RenderClear(gs->renderer);
    render_balls(gs, &gs->grid);
    render_balls(gs, &gs->colonists);
    render_balls(gs, &gs->troops);
    render_selected_troops(gs, gs->paladin);
    render_flags(gs, gs->flag_tex);
    render_balls(gs, &gs->map);
    render_balls(gs, &gs->collision_map);
    render_selecting_rect(gs);
    sprintf(gs->food_label, "Food: %d", gs->res.food);
    sprintf(gs->colonist_label, "Colonists: %d", gs->res.colonists);
    SDL_SetRenderDrawColor(gs->renderer, 0, 255, 0, 255);
    SDL_RenderDebugText(gs->renderer, 14, 65, gs->food_label);
    SDL_RenderDebugText(gs->renderer, 14, 85, gs->colonist_label);
    SDL_RenderPresent(gs->renderer);
}
