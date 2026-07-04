#include "momentum.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

Cell (*grid)[L][L] = NULL;
Cell (*grid_next)[L][L] = NULL;
float (*dist_map)[L][L] = NULL;
int tick = 0;

// Variável global para controlar a visão
static int view_mode = 0;  // 0=isométrica, 1=X, 2=Y, 3=Z

static const int dirs[6][3] = {
    {1,0,0}, {-1,0,0},
    {0,1,0}, {0,-1,0},
    {0,0,1}, {0,0,-1}
};

// Função para inicializar o mapa de distância (provisório)
void init_dist_map(void) {
    dist_map = malloc(sizeof(float) * L * L * L);
    if (!dist_map) exit(1);
    
    // Inicializa com distância euclidiana do centro
    for (int x = 0; x < L; x++)
    for (int y = 0; y < L; y++)
    for (int z = 0; z < L; z++) {
        float dx = (float)(x - MID);
        float dy = (float)(y - MID);
        float dz = (float)(z - MID);
        dist_map[x][y][z] = sqrtf(dx*dx + dy*dy + dz*dz);
    }
}

// Função para liberar o mapa de distância
void free_dist_map(void) {
    if (dist_map) {
        free(dist_map);
        dist_map = NULL;
    }
}

void init_momentum(int xd, int yd, int zd) {
    grid = malloc(sizeof(Cell) * L * L * L);
    grid_next = malloc(sizeof(Cell) * L * L * L);
    if (!grid || !grid_next) exit(1);

    // Inicializa o mapa de distância
    init_dist_map();

    for (int x = 0; x < L; x++)
    for (int y = 0; y < L; y++)
    for (int z = 0; z < L; z++) {
        grid[x][y][z].pB = grid_next[x][y][z].pB = 0;
        grid[x][y][z].dx = grid_next[x][y][z].dx = 0;
        grid[x][y][z].dy = grid_next[x][y][z].dy = 0;
        grid[x][y][z].dz = grid_next[x][y][z].dz = 0;
        grid[x][y][z].r2 = grid_next[x][y][z].r2 = INF_R2;
        grid[x][y][z].dist = grid_next[x][y][z].dist = 0.0f;
        grid[x][y][z].step = grid_next[x][y][z].step = 0;
        grid[x][y][z].total_steps = grid_next[x][y][z].total_steps = 0;
    }

    // Semente com direção arbitrária
    grid[MID][MID][MID].pB = 1;
    grid[MID][MID][MID].dx = (char)xd;
    grid[MID][MID][MID].dy = (char)yd;
    grid[MID][MID][MID].dz = (char)zd;
    grid[MID][MID][MID].dist = 0.0f;
    grid[MID][MID][MID].step = 0;
    
    // Calcula o total de passos baseado na magnitude
    int total = abs(xd) + abs(yd) + abs(zd);
    grid[MID][MID][MID].total_steps = total > 0 ? total : 1;
}

void step_momentum(void) {
    // Primeiro copia o grid
    for (int x = 0; x < L; x++)
    for (int y = 0; y < L; y++)
    for (int z = 0; z < L; z++) {
        grid_next[x][y][z] = grid[x][y][z];
    }

    // Processa cada célula
    for (int x = 1; x < L-1; x++)
    for (int y = 1; y < L-1; y++)
    for (int z = 1; z < L-1; z++) {
        // Se já está ativa, mantém
        if (grid[x][y][z].pB) {
            grid_next[x][y][z].pB = 1;
            continue;
        }

        // Verifica se a distância está preenchida no mapa
        if (dist_map[x][y][z] >= 1e30f) continue;

        // Verifica cada vizinho (von Neumann)
        for (int d = 0; d < 6; d++) {
            int nx = x - dirs[d][0];
            int ny = y - dirs[d][1];
            int nz = z - dirs[d][2];

            if (nx < 0 || nx >= L || ny < 0 || ny >= L || nz < 0 || nz >= L) continue;

            // Verifica se a distância anterior está preenchida
            if (dist_map[nx][ny][nz] >= 1e30f) continue;

            Cell *prev = &grid[nx][ny][nz];
            if (!prev->pB) continue;

            // Obtém a direção original (com magnitude)
            int dx_orig = prev->dx;
            int dy_orig = prev->dy;
            int dz_orig = prev->dz;
            
            // Normaliza a direção para -1, 0, 1
            int norm_dx = (dx_orig > 0) ? 1 : (dx_orig < 0) ? -1 : 0;
            int norm_dy = (dy_orig > 0) ? 1 : (dy_orig < 0) ? -1 : 0;
            int norm_dz = (dz_orig > 0) ? 1 : (dz_orig < 0) ? -1 : 0;

            // Se a direção é zero, não propaga
            if (norm_dx == 0 && norm_dy == 0 && norm_dz == 0) continue;

            // Calcula a magnitude total
            int total_magnitude = abs(dx_orig) + abs(dy_orig) + abs(dz_orig);
            if (total_magnitude == 0) total_magnitude = 1;
            
            // Usa o step da célula anterior para determinar o próximo
            int current_step = prev->step;
            int next_step = (current_step + 1) % total_magnitude;
            
            // Determina qual componente deve ser movida baseado no step
            int component_to_move = -1;
            
            // Distribui os passos proporcionalmente à magnitude
            int dx_mag = abs(dx_orig);
            int dy_mag = abs(dy_orig);
            int dz_mag = abs(dz_orig);
            
            // Acumula as magnitudes para determinar o eixo
            if (next_step < dx_mag) {
                component_to_move = 0;  // X
            } else if (next_step < dx_mag + dy_mag) {
                component_to_move = 1;  // Y
            } else if (next_step < dx_mag + dy_mag + dz_mag) {
                component_to_move = 2;  // Z
            }
            
            if (component_to_move == -1) continue;
            
            // Verifica se o movimento atual corresponde à componente correta
            int correct_move = 0;
            if (component_to_move == 0 && dirs[d][0] == norm_dx) correct_move = 1;
            else if (component_to_move == 1 && dirs[d][1] == norm_dy) correct_move = 1;
            else if (component_to_move == 2 && dirs[d][2] == norm_dz) correct_move = 1;
            
            if (!correct_move) continue;

            // Verifica se a distância está aumentando (propagação para fora)
            float dist_here = dist_map[x][y][z];
            float dist_prev = dist_map[nx][ny][nz];
            
            // A distância deve ser maior que a anterior (propagação para fora)
            if (dist_here <= dist_prev) continue;
            
            // Verifica se a diferença de distância é razoável
            float dist_diff = dist_here - dist_prev;
            if (dist_diff > 2.0f) continue;

            // Propaga!
            grid_next[x][y][z].pB = 1;
            grid_next[x][y][z].dx = prev->dx;
            grid_next[x][y][z].dy = prev->dy;
            grid_next[x][y][z].dz = prev->dz;
            grid_next[x][y][z].dist = dist_here;
            grid_next[x][y][z].step = next_step;
            grid_next[x][y][z].total_steps = total_magnitude;
            break;
        }
    }

    // Troca os grids
    Cell (*tmp)[L][L] = grid;
    grid = grid_next;
    grid_next = tmp;
    tick++;
}

#ifndef NO_SDL
// Função auxiliar para desenhar a retícula
void render_grid(SDL_Renderer *ren, int ox, int oy, int scale) {
    // Desenha linhas da retícula no plano Z=0 (plano XY)
    // A cada 8 células (potência de 2)
    int step = 8;
    
    // Ajusta a escala para a retícula
    float grid_scale = (float)(scale * 0.9f);
    
    SDL_SetRenderDrawColor(ren, 60, 60, 70, 100);  // Cinza escuro com transparência
    
    // Desenha linhas horizontais (paralelas ao eixo X)
    for (int y = -MID; y <= MID; y += step) {
        // Verifica se y está dentro do grid
        int grid_y = y + MID;
        if (grid_y < 0 || grid_y >= L) continue;
        
        // Projeta os pontos da linha no plano isométrico
        int start_x = -MID;
        int end_x = MID;
        
        int sx1 = ox + (int)((float)(start_x - y) * grid_scale);
        int sy1 = oy + (int)((float)(start_x + y) * grid_scale * 0.45f);
        int sx2 = ox + (int)((float)(end_x - y) * grid_scale);
        int sy2 = oy + (int)((float)(end_x + y) * grid_scale * 0.45f);
        
        SDL_RenderLine(ren, sx1, sy1, sx2, sy2);
    }
    
    // Desenha linhas verticais (paralelas ao eixo Y)
    for (int x = -MID; x <= MID; x += step) {
        int grid_x = x + MID;
        if (grid_x < 0 || grid_x >= L) continue;
        
        int start_y = -MID;
        int end_y = MID;
        
        int sx1 = ox + (int)((float)(x - start_y) * grid_scale);
        int sy1 = oy + (int)((float)(x + start_y) * grid_scale * 0.45f);
        int sx2 = ox + (int)((float)(x - end_y) * grid_scale);
        int sy2 = oy + (int)((float)(x + end_y) * grid_scale * 0.45f);
        
        SDL_RenderLine(ren, sx1, sy1, sx2, sy2);
    }
}

// Função para projetar coordenadas 3D na tela
void project_point(int x, int y, int z, int ox, int oy, int scale, int *sx, int *sy) {
    int dx = x - MID;
    int dy = y - MID;
    int dz = z - MID;
    
    float s = (float)(scale * 0.9f);
    
    switch(view_mode) {
        case 0:  // Isométrica
            *sx = ox + (int)((float)(dx - dy) * s);
            *sy = oy - (int)((float)dz * s) + (int)((float)(dx + dy) * s * 0.45f);
            break;
        case 1:  // Visão X (plano YZ)
            *sx = ox + (int)((float)dy * s);
            *sy = oy - (int)((float)dz * s);
            break;
        case 2:  // Visão Y (plano XZ)
            *sx = ox + (int)((float)dx * s);
            *sy = oy - (int)((float)dz * s);
            break;
        case 3:  // Visão Z (plano XY)
            *sx = ox + (int)((float)dx * s);
            *sy = oy - (int)((float)dy * s);
            break;
    }
}

void render_momentum(SDL_Renderer *ren)
{
    const int WIN_W = 1280;
    const int WIN_H = 800;

    const float scale_factor = 0.9f;
    const int scale = (int)(4 * scale_factor);

    const int ox = WIN_W / 2;
    const int oy = WIN_H / 2 - 50;

    // Limpa a tela
    SDL_SetRenderDrawColor(ren, 12, 12, 20, 255);
    SDL_RenderClear(ren);
    
    // Desenha a retícula no plano XY (apenas na visão isométrica ou Z)
    if (view_mode == 0 || view_mode == 3) {
        render_grid(ren, ox, oy, scale);
    }
    
    // Desenha as células ativas
    for (int s = 0; s <= 3*(L-1); s++)
    {
        for (int x = 0; x < L; x++)
        for (int y = 0; y < L; y++)
        {
            int z = s - x - y;

            if (z < 0 || z >= L) continue;
            if (!grid[x][y][z].pB) continue;

            int sx, sy;
            project_point(x, y, z, ox, oy, scale, &sx, &sy);

            // Cor baseada na direção
            Cell *cell = &grid[x][y][z];
            Uint8 r = 0, g = 0, b = 0;
            
            if (cell->dx > 0) r = (Uint8)(255);
            else if (cell->dx < 0) r = (Uint8)(128);
            
            if (cell->dy > 0) g = (Uint8)(255);
            else if (cell->dy < 0) g = (Uint8)(128);
            
            if (cell->dz > 0) b = (Uint8)(255);
            else if (cell->dz < 0) b = (Uint8)(128);

            SDL_SetRenderDrawColor(ren, r, g, b, 255);

            SDL_FRect rect = {
                (float)sx,
                (float)sy,
                (float)scale,
                (float)scale
            };

            SDL_RenderFillRect(ren, &rect);
        }
    }

    // Mostra o modo de visão atual (usando um retângulo simples)
    SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
    SDL_FRect text_bg = {10, 10, 200, 30};
    SDL_RenderFillRect(ren, &text_bg);
    
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderLine(ren, 10, 10, 210, 10);
    SDL_RenderLine(ren, 10, 10, 10, 40);
    SDL_RenderLine(ren, 210, 10, 210, 40);
    SDL_RenderLine(ren, 10, 40, 210, 40);

    // Eixos (apenas na visão isométrica)
    if (view_mode == 0) {
        SDL_SetRenderDrawColor(ren,220,220,220,255);

        // Eixo X (vermelho)
        SDL_SetRenderDrawColor(ren,255,80,80,255);
        SDL_RenderLine(ren, ox, oy, ox + (int)((L/2)*scale*0.9f), oy + (int)((L/4)*scale*0.9f));

        // Eixo Y (verde)
        SDL_SetRenderDrawColor(ren,80,255,80,255);
        SDL_RenderLine(ren, ox, oy, ox, oy - (int)((L/2)*scale*0.9f));

        // Eixo Z (azul)
        SDL_SetRenderDrawColor(ren,80,160,255,255);
        SDL_RenderLine(ren, ox, oy, ox - (int)((L/2)*scale*0.9f), oy + (int)((L/4)*scale*0.9f));
    } else if (view_mode == 1) {
        // Eixos para visão X (plano YZ)
        SDL_SetRenderDrawColor(ren,80,255,80,255);
        SDL_RenderLine(ren, ox, oy, ox + (int)((L/2)*scale*0.9f), oy);
        SDL_SetRenderDrawColor(ren,80,160,255,255);
        SDL_RenderLine(ren, ox, oy, ox, oy - (int)((L/2)*scale*0.9f));
    } else if (view_mode == 2) {
        // Eixos para visão Y (plano XZ)
        SDL_SetRenderDrawColor(ren,255,80,80,255);
        SDL_RenderLine(ren, ox, oy, ox + (int)((L/2)*scale*0.9f), oy);
        SDL_SetRenderDrawColor(ren,80,160,255,255);
        SDL_RenderLine(ren, ox, oy, ox, oy - (int)((L/2)*scale*0.9f));
    } else if (view_mode == 3) {
        // Eixos para visão Z (plano XY)
        SDL_SetRenderDrawColor(ren,255,80,80,255);
        SDL_RenderLine(ren, ox, oy, ox + (int)((L/2)*scale*0.9f), oy);
        SDL_SetRenderDrawColor(ren,80,255,80,255);
        SDL_RenderLine(ren, ox, oy, ox, oy + (int)((L/2)*scale*0.9f));
    }
}

int main(void) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window =
        SDL_CreateWindow(
            "Momentum - Isometric",
            1280,
            800,
            0);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    // Teste com direção (0, -5, 3)
    init_momentum(0, 5, 3);

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = 0;
            
            // Teclas para mudar a visão
            if (event.type == SDL_EVENT_KEY_DOWN) {
                switch(event.key.key) {
                    case SDLK_X:
                        view_mode = 1;  // Visão X
                        break;
                    case SDLK_Y:
                        view_mode = 2;  // Visão Y
                        break;
                    case SDLK_Z:
                        view_mode = 3;  // Visão Z
                        break;
                    case SDLK_I:
                        view_mode = 0;  // Visão Isométrica
                        break;
                }
            }
        }

        step_momentum();
        render_momentum(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(50);
    }

    free(grid);
    free(grid_next);
    free_dist_map();
    SDL_Quit();
    return 0;
}
#endif