#include "game_logic.h"
#include <SDL3_image/SDL_image.h>

size_t get_tile_index(game_state* gs, int x, int y) {
    int tile_x = x / gs->tile_width;
    int tile_y = y / gs->tile_height;
    return tile_y * gs->tiles_x + tile_x;
}

void draw_grid(game_state* gs, uint16_t width, uint16_t height) {
    gs->tiles_x = ((WINDOW_WIDTH / width) + 1);
    gs->tiles_y = ((WINDOW_HEIGHT / height) + 1);
    gs->grid_width = gs->tiles_x * width;
    gs->grid_height = gs->tiles_y * height;
    color ca = {0, 50, 0, 50};
    color cb = {0, 0, 50, 50};
    color c;
    for (size_t y = 0; y < gs->tiles_y; y++) {
        for (size_t x = 0; x < gs->tiles_x; x++) {
            if ((x % 2 + y % 2) % 2) { c = ca; } else { c = cb; }
            spawn_ball(x * width, y * height, gs->tile_width, gs->tile_height, &gs->grid, c, NULL);
        }
    }
}

void draw_house(game_state* gs, uint16_t x, uint16_t y, uint16_t house_width, uint16_t house_height, uint16_t door_pos, SDL_Texture* tex) {
    uint16_t tx = x * gs->tile_width;
    uint16_t ty = y * gs->tile_height;
    for (int w = 0; w < house_width + 1; w++) {
        spawn_ball(tx + w * gs->tile_width, ty, gs->tile_width, gs->tile_height, &gs->map, no_color, tex);
        spawn_ball(tx + w * gs->tile_width, ty + house_height * gs->tile_height, gs->tile_width, gs->tile_height, &gs->map, no_color, tex);
    }
    for (int h = 0; h < house_height + 1; h++) {
        spawn_ball(tx, ty + h * gs->tile_height, gs->tile_width, gs->tile_height, &gs->map, no_color, tex);
        spawn_ball(tx + house_width * gs->tile_width, ty + h * gs->tile_height, gs->tile_width, gs->tile_height, &gs->map, no_color, tex);
    }
    spawn_ball(tx, ty, (house_width + 1) * gs->tile_width, gs->tile_height, &gs->collision_map, black, NULL);
    spawn_ball(tx, ty + house_height * gs->tile_width, (house_width + 1) * gs->tile_width, gs->tile_height, &gs->collision_map, no_color, NULL);
    spawn_ball(tx, ty, gs->tile_width, gs->tile_height * house_height, &gs->collision_map, no_color, gs->paladin);
    spawn_ball(tx + gs->tile_width * house_width, ty, gs->tile_width, gs->tile_height * house_height, &gs->collision_map, no_color, gs->human);
}

void switch_color(ball* tile) {
    color ca = {0, 50, 0, 50};
    color cb = {0, 0, 50, 50};
    if (tile->color.g == 50) {
        tile->color = cb;
    } else {
        tile->color = ca;
    }
}

void move_map(game_state* gs, int dx, int dy) {
    size_t maxm = (*gs->movables[1])->num;
    for (size_t i = 2; i < MQ; i++) {
        if ((*gs->movables[i])->num > maxm) {
            maxm = (*gs->movables[i])->num;
        }
    }
    for (size_t j = 1; j < MQ; j++) {
        for (size_t i = 0; i < maxm; i++) {
            if (i < (*gs->movables[j])->num) {
                (*gs->movables[j])->arr[i].pos.x += dx;
                (*gs->movables[j])->arr[i].pos.y += dy;
            }
        }
    }
    uint8_t reset_x = 0;
    uint8_t reset_y = 0;
    gs->move_map_counter_x += dx;
    gs->move_map_counter_y += dy;
    bl* grid_bl = *gs->movables[0];
    for (size_t i = 0; i < grid_bl->num; i++) {
        grid_bl->arr[i].pos.x += dx;
        grid_bl->arr[i].pos.y += dy;
        if (gs->move_map_counter_x >= gs->tile_width) {
            grid_bl->arr[i].pos.x -= gs->tile_width;
            switch_color(&grid_bl->arr[i]);
            reset_x = 1;
        } else if (gs->move_map_counter_x <= -gs->tile_width) {
            grid_bl->arr[i].pos.x += gs->tile_width;
            switch_color(&grid_bl->arr[i]);
            reset_x = 1;
        }
        if (gs->move_map_counter_y >= gs->tile_height) {
            grid_bl->arr[i].pos.y -= gs->tile_height;
            switch_color(&grid_bl->arr[i]);
            reset_y = 1;
        } else if (gs->move_map_counter_y <= -gs->tile_height) {
            grid_bl->arr[i].pos.y += gs->tile_height;
            switch_color(&grid_bl->arr[i]);
            reset_y = 1;
        }
    }
    if (reset_x) { gs->move_map_counter_x = 0; }
    if (reset_y) { gs->move_map_counter_y = 0; }
}

void reset_flags(game_state* gs, uint8_t squad_num) {
    vec2D origin = {0, 0};
    for (uint8_t i = 0; i < FLAG_LIMIT; i++) {
        gs->army[squad_num].flags[i] = origin;
    }
}

void set_flag(game_state* gs, vec2D flag) {
    formation* squat = &gs->army[gs->formation_selector];
    squat->flags[squat->last_flag] = flag;
    squat->last_flag = (squat->last_flag + 1) % FLAG_LIMIT;
}

void move_colonists(game_state* gs) {
    // Implement logic here if needed
}

void move_troop(game_state* gs, uint16_t selected_troop, float troop_velo, vec2D flag) {
    ball* t = &gs->troops->arr[selected_troop];
    vec2D fv = dvec(t->pos, flag);
    vec2D fvn = normalize(fv);
    t->pos.x += troop_velo * fvn.x;
    t->pos.y += troop_velo * fvn.y;
}

void move_army(game_state* gs, float troop_velo) {
    formation* squat;
    vec2D flag;
    ball* t;
    vec2D origin = {0, 0};
    for (uint8_t i = 0; i < 10; i++) {
        squat = &gs->army[i];
        flag = squat->flags[squat->curr_flag];
        if (flag.x != 0 && flag.y != 0) {
            for (uint8_t j = 0; j < squat->num; j++) {
                move_troop(gs, squat->troops[j], troop_velo, flag);
                t = &gs->troops->arr[squat->troops[j]];
                if (magnitude(dvec(t->pos, flag)) < 1) {
                    squat->flags[squat->curr_flag] = origin;
                    squat->curr_flag = (squat->curr_flag + 1) % FLAG_LIMIT;
                }
            }
        }
    }
}

void initialize_formations(game_state* gs) {
    for (uint8_t i = 0; i < 10; i++) {
        gs->army[i].num = 0;
    }
}

void update(game_state* gs) {
    move_colonists(gs);
    move_army(gs, 1);
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - gs->last_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    gs->delta_time = (SDL_GetTicks() - gs->last_frame_time) / 1000.0f;
    gs->last_frame_time = SDL_GetTicks();
}
