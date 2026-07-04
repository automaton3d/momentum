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
    unsigned int r2;
    unsigned char active;
    unsigned char pB;
    char dx, dy, dz;        // direção da célula (com magnitude)
    float dist;             // distância da célula no mapa
    int step;               // passo atual na sequência de movimento
    int total_steps;        // total de passos para completar um ciclo
} Cell;

extern Cell (*grid)[L][L];
extern Cell (*grid_next)[L][L];
extern float (*dist_map)[L][L];
extern int tick;

void init_momentum(int xd, int yd, int zd);   // direção inicial
void step_momentum(void);
void init_dist_map(void);
void free_dist_map(void);

#ifndef NO_SDL
void render_momentum(SDL_Renderer *ren);
#endif

#endif