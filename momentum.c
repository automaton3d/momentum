#include "momentum.h"
#include <stdlib.h>
#include <string.h>

Cell (*grid)[L][L] = NULL;
Cell (*grid_next)[L][L] = NULL;
int tick = 0;

void init_momentum(void) {
    grid      = malloc(sizeof(Cell) * L * L * L);
    grid_next = malloc(sizeof(Cell) * L * L * L);
    if (!grid || !grid_next) exit(1);

    for (int x = 0; x < L; x++)
    for (int y = 0; y < L; y++)
    for (int z = 0; z < L; z++) {
        grid[x][y][z].pB = grid_next[x][y][z].pB = 0;
        grid[x][y][z].r2 = grid_next[x][y][z].r2 = INF_R2;
    }

    // Semente inicial no centro
    grid[MID][MID][MID].pB = 1;
}

static const int dirs[6][3] = {
    {1,0,0}, {-1,0,0},
    {0,1,0}, {0,-1,0},
    {0,0,1}, {0,0,-1}
};

void step_momentum(void) {
    for (int x = 0; x < L; x++)
    for (int y = 0; y < L; y++)
    for (int z = 0; z < L; z++) {
        grid_next[x][y][z] = grid[x][y][z];
    }

    for (int x = 1; x < L-1; x++)
    for (int y = 1; y < L-1; y++)
    for (int z = MID; z < L-1; z++) {          // só acima do centro
        if (grid[x][y][z].pB) {
            grid_next[x][y][z].pB = 1;         // mantém-se
            continue;
        }

        // Regra muito restrita para linha fina:
        // Ativa apenas se:
        // - Tem vizinho imediatamente abaixo (z-1)
        // - E está alinhado com o centro (x,y próximos de MID)
        int has_below = (grid[x][y][z-1].pB);

        int aligned = (abs(x - MID) == 0 && abs(y - MID) == 0);

        if (has_below && aligned) {
            grid_next[x][y][z].pB = 1;
        }
    }

    Cell (*tmp)[L][L] = grid;
    grid = grid_next;
    grid_next = tmp;
    tick++;
}

#ifndef NO_SDL
void render_momentum(SDL_Renderer *ren)
{
    const int WIN_W = 1280;
    const int WIN_H = 800;

    const int scale = 4;

    const int ox = WIN_W / 2;
    const int oy = WIN_H / 2;

    SDL_SetRenderDrawColor(ren, 12,12,20,255);
    SDL_RenderClear(ren);
    
    //-------------------------------------------------
    // Percorre TODA a grade
    //-------------------------------------------------

    for (int s = 0; s <= 3*(L-1); s++)
    {
        for (int x = 0; x < L; x++)
        for (int y = 0; y < L; y++)
        {
            int z = s - x - y;

            if (z < 0 || z >= L)
                continue;

            if (!grid[x][y][z].pB)
                continue;

            //------------------------------------------
            // projeção isométrica
            //------------------------------------------

            int dx = x - MID;
            int dy = y - MID;
            int dz = z - MID;

            int sx = ox + (dx - dy) * scale;
            int sy = oy - dz * scale + (dx + dy) * scale / 2;
            //------------------------------------------
            // cor depende da profundidade
            //------------------------------------------

            Uint8 g = 40 + 215*z/L;

            SDL_SetRenderDrawColor(
                ren,
                0,
                g,
                255,
                255);

            SDL_FRect r =
            {
                (float)sx,
                (float)sy,
                (float)scale,
                (float)scale
            };

            SDL_RenderFillRect(ren,&r);
        }
    }

    //-------------------------------------------------
    // desenha os três eixos
    //-------------------------------------------------

    SDL_SetRenderDrawColor(ren,220,220,220,255);

//-------------------------------------------------
// eixo X (vermelho)
//-------------------------------------------------

SDL_SetRenderDrawColor(ren,255,80,80,255);

SDL_RenderLine(
    ren,
    ox,
    oy,
    ox + (L/2)*scale,
    oy + (L/4)*scale);

//-------------------------------------------------
// eixo Y (verde)
//-------------------------------------------------

SDL_SetRenderDrawColor(ren,80,255,80,255);

SDL_RenderLine(
    ren,
    ox,
    oy,
    ox,
    oy - (L/2)*scale);

//-------------------------------------------------
// eixo Z (azul)
//-------------------------------------------------

SDL_SetRenderDrawColor(ren,80,160,255,255);

SDL_RenderLine(
    ren,
    ox,
    oy,
    ox - (L/2)*scale,
    oy + (L/4)*scale);
}

int main(void) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window =
        SDL_CreateWindow(
            "Momentum - Isometric",
            1280,   // 1600 × 0.8
            800,    // 1000 × 0.8
            0);

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