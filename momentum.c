#include "momentum.h"
#include <stdlib.h>

Cell (*grid)[L][L] = NULL;
Cell (*grid_next)[L][L] = NULL;
int tick = 0;

void init_momentum(void) {
    grid = malloc(sizeof(Cell) * L * L * L);
    grid_next = malloc(sizeof(Cell) * L * L * L);
    if (!grid || !grid_next) exit(1);

    for (int x = 0; x < L; x++)
    for (int y = 0; y < L; y++)
    for (int z = 0; z < L; z++) {
        grid[x][y][z].pB = grid_next[x][y][z].pB = 0;
        grid[x][y][z].r2 = grid_next[x][y][z].r2 = INF_R2;
    }
    grid[MID][MID][MID].pB = 1;
}

void step_momentum(void) {
    for (int x = 0; x < L; x++)
    for (int y = 0; y < L; y++)
    for (int z = 0; z < L; z++) {
        grid_next[x][y][z] = grid[x][y][z];
    }

    int z_pos = MID + (tick / 4);   // crescimento lento e controlado
    if (z_pos < L) {
        grid_next[MID][MID][z_pos].pB = 1;
    }

    Cell (*tmp)[L][L] = grid;
    grid = grid_next;
    grid_next = tmp;
    tick++;
}

#ifndef NO_SDL
void render_momentum(SDL_Renderer *ren) {
    SDL_SetRenderDrawColor(ren, 10, 10, 30, 255);
    SDL_RenderClear(ren);

    const int scale = 2;
    const int ox = 150;
    const int oy = 100;

    int cx = ox + MID * scale;
    int cy = oy + MID * scale;

    // Slice XY central
    for (int x = 0; x < L; x += 1)
    for (int y = 0; y < L; y += 1) {
        if (grid[x][y][MID].pB) {
            SDL_SetRenderDrawColor(ren, 0, 255, 255, 255);
            SDL_FRect r = {(float)(ox + x*scale), (float)(oy + y*scale), (float)scale, (float)scale};
            SDL_RenderFillRect(ren, &r);
        }
    }

    // Eixos cruzando o centro
    SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
    SDL_RenderLine(ren, (float)ox, (float)cy, (float)(ox + L*scale), (float)cy);           // X
    SDL_RenderLine(ren, (float)cx, (float)oy, (float)cx, (float)(oy + L*scale));           // Y

    // Linha Z lateral (vertical)
    int sx = ox + L*scale + 80;
    SDL_SetRenderDrawColor(ren, 0, 240, 255, 255);
    for (int i = 0; i <= (tick / 4); i++) {
        if (MID + i >= L) break;
        SDL_FRect r = {(float)sx, (float)(oy + i*scale), 25.0f, (float)scale};
        SDL_RenderFillRect(ren, &r);
    }
}

int main(void) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Momentum - Linha pB no eixo Z", 1600, 1000, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    init_momentum();

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = 0;
        }

        step_momentum();
        render_momentum(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(50);
    }

    free(grid);
    free(grid_next);
    SDL_Quit();
    return 0;
}
#endif