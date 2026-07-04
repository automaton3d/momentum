#ifndef MOMENTUM_H_
#define MOMENTUM_H_

#include <stdint.h>
#ifndef NO_SDL
#include <SDL3/SDL.h>
#endif

#define L          221
#define MID        (L / 2)
#define INF_R2     0xFFFFFFFFu

typedef struct {
    unsigned int r2;      // distância² (compatibilidade com wavefront)
    unsigned char active;
    unsigned char pB;     // bit momentum / partícula B
} Cell;

extern Cell (*grid)[L][L];
extern Cell (*grid_next)[L][L];
extern int tick;

void init_momentum(void);
void step_momentum(void);

#ifndef NO_SDL
void render_momentum(SDL_Renderer *ren);
#endif

#endif