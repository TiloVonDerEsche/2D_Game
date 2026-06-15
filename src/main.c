#include <stdio.h>
#include "game.h"
#include "renderer.h"
#include "input_handler.h"
#include "game_logic.h"
#include "utils.h"

char* separator = "/";

game_state gs = {
    .window_title = "2D Game",
    .game_is_running = FALSE,

    .delta_time = 0,
    .last_frame_time = 0,

    .window = NULL,
    .renderer = NULL,

    .brick = NULL,
    .dirt = NULL,
    .painting = NULL,
    .flag_tex = NULL,
    .paladin = NULL,
    .guard = NULL,
    .human = NULL,

    .food_label = NULL,
    .colonist_label = NULL,

    .spawn_density = 1.0,

    .grid = NULL,
    .colonists = NULL,
    .troops = NULL,
    .map = NULL,
    .collision_map = NULL,

    .selected_troops = NULL,
    .selected_colonists = NULL,
    .selected_troop_num = 0,
    .selected_colonist_num = 0,
    .formation_selector = 0,

    .flag_limit = 200,
    .troop_limit = 100,
    .colonist_limit = 100,

    .flag_width = 30,
    .flag_height = 30,
    .troop_width = 50,
    .troop_height = 50,
    .colonist_width = 50,
    .colonist_height = 50,
    .tile_width = 50,
    .tile_height = 50,
    .grid_width = 0,
    .grid_height = 0,

    .tiles_x = 0,
    .tiles_y = 0,

    .move_map_counter_x = 0,
    .move_map_counter_y = 0,
    .dragging = 0,
    .follow_flag = 0,

    .res = {0, 0},
    .mouse = {0, 0},
    .flag = {0, 0},
    .drag_start = {0, 0}
};

color no_color = {0, 0, 0, 0};
color black = {0, 0, 0, 255};
color red = {255, 0, 0, 255};
color green = {0, 255, 0, 100};
color blue = {0, 0, 255, 255};
color yellow = {255, 255, 0, 255};
color white = {255, 255, 255, 255};

int initialize_window(game_state* gs) {
    int initErrC = SDL_Init(SDL_INIT_VIDEO);

    if (initErrC != 1) {
        fprintf(stderr, "SDL Init Error: %s\n", SDL_GetError());
        return FALSE;
    }

    gs->window = SDL_CreateWindow(gs->window_title, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_FULLSCREEN);

    if (!gs->window) {
        fprintf(stderr, "Window Creation Error: %s\n", SDL_GetError());
        return FALSE;
    }

    gs->renderer = SDL_CreateRenderer(gs->window, "gpu");

    if (!gs->renderer) {
        fprintf(stderr, "Renderer Creation Error: %s\n", SDL_GetError());
        return FALSE;
    }

    return TRUE;
}

void setup(game_state* gs) {
    char base[512];
    char path[512];
    build_path(base, sizeof(base), separator, 2, "src", "assets");

    build_path(path, sizeof(path), separator, 3, base, "tileable_brick_ground_textures", "Ground_03.png");
    gs->brick = IMG_LoadTexture(gs->renderer, path);

    build_path(path, sizeof(path), separator, 3, base, "tileable_dirt_textures", "Dirt_03.png");
    gs->dirt = IMG_LoadTexture(gs->renderer, path);

    build_path(path, sizeof(path), separator, 3, base, "tileable_dirt_textures", "painting-27.jpg");
    gs->painting = IMG_LoadTexture(gs->renderer, path);

    build_path(path, sizeof(path), separator, 3, base, "medieval_signs", "torch.png");
    gs->flag_tex = IMG_LoadTexture(gs->renderer, path);

    build_path(path, sizeof(path), separator, 3, base, "entity", "paladin.png");
    gs->paladin = IMG_LoadTexture(gs->renderer, path);

    build_path(path, sizeof(path), separator, 3, base, "entity", "vault_guard.png");
    gs->guard = IMG_LoadTexture(gs->renderer, path);

    build_path(path, sizeof(path), separator, 3, base, "entity", "human.png");
    gs->human = IMG_LoadTexture(gs->renderer, path);

    gs->grid = malloc(sizeof(bl) + 5 * sizeof(ball));
    gs->grid->num = 1;
    gs->grid->len = 5;
    draw_grid(gs, gs->tile_width, gs->tile_height);

    gs->colonists = malloc(sizeof(bl) + 10 * sizeof(ball));
    gs->colonists->num = 0;
    gs->colonists->len = 10;
    spawn_ball(500, 500, gs->colonist_width, gs->colonist_height, &gs->colonists, red, gs->human);
    spawn_ball(550, 500, gs->colonist_width, gs->colonist_height, &gs->colonists, red, gs->human);
    spawn_ball(600, 500, gs->colonist_width, gs->colonist_height, &gs->colonists, red, gs->human);
    gs->res.colonists = 3;

    gs->troops = malloc(sizeof(bl) + 10 * sizeof(ball));
    gs->troops->num = 0;
    gs->troops->len = 10;
    spawn_ball(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, gs->troop_width, gs->troop_height, &gs->troops, red, gs->guard);
    spawn_ball(WINDOW_WIDTH / 2 + 250, WINDOW_HEIGHT / 2, gs->troop_width, gs->troop_height, &gs->troops, red, gs->guard);
    spawn_ball(WINDOW_WIDTH / 2 + 500, WINDOW_HEIGHT / 2, gs->troop_width, gs->troop_height, &gs->troops, red, gs->guard);
    spawn_ball(WINDOW_WIDTH / 2 + 750, WINDOW_HEIGHT / 2, gs->troop_width, gs->troop_height, &gs->troops, red, gs->guard);
    spawn_ball(WINDOW_WIDTH / 2 + 1000, WINDOW_HEIGHT / 2, gs->troop_width, gs->troop_height, &gs->troops, red, gs->guard);

    gs->selected_colonists = malloc(COLONIST_LIMIT * sizeof(uint16_t));
    gs->selected_troops = malloc(TROOP_LIMIT * sizeof(uint16_t));
    gs->selected_troop_num = 0;

    initialize_formations(gs);

    gs->map = malloc(sizeof(bl) + 10 * sizeof(ball));
    gs->map->num = 0;
    gs->map->len = 10;

    gs->collision_map = malloc(sizeof(bl) + 10 * sizeof(ball));
    gs->collision_map->num = 0;
    gs->collision_map->len = 10;

    gs->movables[0] = &gs->grid;
    gs->movables[1] = &gs->colonists;
    gs->movables[2] = &gs->troops;
    gs->movables[3] = &gs->map;
    gs->movables[4] = &gs->collision_map;

    gs->food_label = calloc(6 + sizeof(int), 1);
    gs->colonist_label = calloc(11 + sizeof(float), 1);

    draw_house(gs, 1, 1, 4, 3, 4, gs->brick);
    draw_house(gs, 16, 14, 8, 6, 5, gs->brick);
}

void destroy_window(game_state* gs) {
    free(gs->colonist_label);
    free(gs->food_label);
    free(gs->grid);
    free(gs->colonists);
    free(gs->troops);
    free(gs->selected_colonists);
    free(gs->selected_troops);
    free(gs->map);
    free(gs->collision_map);
    SDL_DestroyRenderer(gs->renderer);
    SDL_DestroyWindow(gs->window);
    SDL_Quit();
}

void game() {
    printf("Game is running...\n");
    gs.game_is_running = initialize_window(&gs);
    setup(&gs);
    while (gs.game_is_running) {
        process_input(&gs);
        update(&gs);
        render(&gs);
    }
    destroy_window(&gs);
}

int main(int argc, char* args[]) {
    game();
    return 0;
}
