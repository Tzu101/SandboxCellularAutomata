#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>


#define width (SCREEN_WIDTH / PARTICLE_SIZE)
#define height (SCREEN_HEIGHT / PARTICLE_SIZE)
#define length 11


// Window constants
const int MENU_HEIGHT = 100;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int FPS = 100;

// Particle constants
const int PARTICLE_SIZE = 5;
const int PARTICLE_UPDATE_DELAY = 1;

const int PARTICLE_VOID = -1;
const int PARTICLE_GAS = 0;
const int PARTICLE_LIQUID = 1;
const int PARTICLE_SOLID = 2;

const int PARTICLE_NONE = -1;
const int PARTICLE_SAND = 0;
const int PARTICLE_DIRT = 1;
const int PARTICLE_STONE = 2;
const int PARTICLE_OBSIDIAN = 3;
const int PARTICLE_STEEL = 4;
const int PARTICLE_WOOD = 5;
const int PARTICLE_WATER = 6;
const int PARTICLE_LAVA = 7;
const int PARTICLE_ACID = 8;
const int PARTICLE_STEAM = 9;
const int PARTICLE_SMOKE= 10;
const int COLORS[length][3] = {{230, 120, 0}, {150, 90, 30}, {70, 75, 70}, {20, 15, 15}, {100, 115, 115}, {120, 60, 0}, {35,137,218}, {255, 42, 0}, {34, 204, 0}, {200, 200, 210}, {10, 5, 5}};
const int TYPES[length] = {2, 2, 2, 2, 2, 2, 1, 1, 1, 0, 0};
const int SPREAD[length] = {3, 2, 1, 0, 0, 0, 2, 1, 1, 2, 1};

// Pre-define Structure
typedef struct Particle Particle;

// Function variables type
typedef void (*function)(Particle*, Particle***, int, int);

// Custum particle struct
struct Particle {
    int type;
    int element;
    int velocity;
    float gravity;
    bool updated;
};
// Creates a particle
void new_particle(Particle* p, int type, int element) {
    p->type = type;
    p->element = 1;
    p->velocity = rand() % 2 == 0 ? -1 : 1;
    p->gravity = 1.0;
    p->updated = false;
}

void mix_elements(Particle*** map, int x1, int y1, int x2, int y2) {

    Particle* p = map[x1][y1];
    map[x1][y1] = map[x2][y2];
    map[x2][y2] = p;
}

bool float_up(Particle* p, Particle*** map, int x, int y) {

    int gravity = (int)p->gravity;
    int yn = y;

    for (int g=1; g<=gravity; g++) {

        if (y-g >= 0 && map[x][y-g]->type < p->type) yn = y-g;
        else break;
    }

    if (y == yn) {
        p->gravity = 1.0;

        return false;
    }
    else {
        mix_elements(map, x, y, x, yn);
        
        p->gravity += 0.1 * (y-yn);

        return true;
    }
}

bool fall_down(Particle* p, Particle*** map, int x, int y) {

    int gravity = (int)p->gravity;
    int yn = y;

    for (int g=1; g<=gravity; g++) {

        if (y+g < height && map[x][y+g]->type < p->type) yn = y+g;
        else break;
    }

    if (y == yn) {
        p->gravity = 1.0;

        return false;
    }
    else {
        mix_elements(map, x, y, x, yn);
        
        p->gravity += 0.1 * (yn-y);

        return true;
    }
}

bool move_left(Particle* p, Particle*** map, int x, int y, int h) {

    int distance = SPREAD[p->element];
    for (int d=0; d < distance; d++) {

        if (x-d > 0) {

            int xn = x-1-d;
            int yn = y+h;

            if (map[xn][y]->type - distance + d + 2 < p->type) {
                if (map[xn][yn]->type < p->type) {
                    
                    mix_elements(map, x, y, xn, yn);
                    
                    return true;
                }
            }
            else break;
        }
    }
    return false;
}

bool move_right(Particle* p, Particle*** map, int x, int y, int h) {

    int distance = SPREAD[p->element];
    for (int d=0; d < distance; d++) {

        if (x+d < width-1) {

            int xn = x+1+d;
            int yn = y+h;

            if (map[xn][y]->type - distance + d + 2 < p->type) {
                if (map[xn][yn]->type < p->type) {

                    mix_elements(map, x, y, xn, yn);

                    return true;
                }
            }
            else break;
        }
    }
    return false;
}

bool move_side(Particle* p, Particle*** map, int x, int y, int h) {

    if ((y < height-1 && h == 1) || (y > 0 && h == -1)) {
        int fall = rand() % 2;

        if (fall == 0) {
            if (move_left(p, map, x, y, h)) return true;
            if (move_right(p, map, x, y, h)) return true;
        }
        else {
            if (move_right(p, map, x, y, h)) return true;
            if (move_left(p, map, x, y, h)) return true;
        }
    }
    return false;
}

bool flow(Particle* p, Particle*** map, int x, int y) {

    int xn; 
    if (p->velocity == 1) {

        xn = x;
        for (int n=1; n<SPREAD[p->element]+1; n++) {

            if (x-n >= 0 && map[x-n][y]->type < p->type) xn = x-n;
            else break;
        }
    }
    if (p->velocity == -1) {

        xn = x;
        for (int n=1; n<SPREAD[p->element]+1; n++) {

            if (x+n < width && map[x+n][y]->type < p->type) xn = x+n;
            else break;
        }
    }
    if (xn == x) {
        p->velocity *= -1;
        return false;
    }
    else {
        mix_elements(map, x, y, xn, y);
        return true;
    }
}

void sand_update(Particle* p, Particle*** map, int x, int y) {

    if (!fall_down(p, map, x, y)) {

        move_side(p, map, x, y, 1);
    }
}

void dirt_update(Particle* p, Particle*** map, int x, int y) {

    if (!fall_down(p, map, x, y)) {

        move_side(p, map, x, y, 1);
    }
}

void stone_update(Particle* p, Particle*** map, int x, int y) {

    if (!fall_down(p, map, x, y)) {

        move_side(p, map, x, y, 1);
    }
}

void obsidian_update(Particle* p, Particle*** map, int x, int y) {
    fall_down(p, map, x, y);
}

void steel_update(Particle* p, Particle*** map, int x, int y) {}

void wood_update(Particle* p, Particle*** map, int x, int y) {}

void water_update(Particle* p, Particle*** map, int x, int y) {

    bool end = false;

    if (x > 0) {
        if (map[x-1][y]->element == PARTICLE_ACID) {
            map[x-1][y]->element = PARTICLE_NONE;
            map[x-1][y]->type = PARTICLE_VOID;
            end = true;
        }
    }
    if (x < width-1) {
        if (map[x+1][y]->element == PARTICLE_ACID) {
            map[x+1][y]->element = PARTICLE_NONE;
            map[x+1][y]->type = PARTICLE_VOID;
            end = true;
        }
    }
    if (y < height-1) {
        if (map[x][y+1]->element == PARTICLE_LAVA) {
            map[x][y+1]->element = PARTICLE_OBSIDIAN;
            map[x][y+1]->type = TYPES[PARTICLE_OBSIDIAN];
            end = true;
        }
        else if (map[x][y+1]->element == PARTICLE_ACID) {
            map[x][y+1]->element = PARTICLE_NONE;
            map[x][y+1]->type = PARTICLE_VOID;
            end = true;
        }
    }
    if (end) {
        p->element = PARTICLE_STEAM;
        p->type = TYPES[PARTICLE_STEAM];
        return;
    }

    if (fall_down(p, map, x, y)) return;
    if (flow(p, map, x, y)) return;
}

void lava_update(Particle* p, Particle*** map, int x, int y) {

    bool end = false;

    if (x > 0) {
        if (map[x-1][y]->element == PARTICLE_WATER || map[x-1][y]->element == PARTICLE_ACID) {
            map[x-1][y]->element = PARTICLE_STEAM;
            map[x-1][y]->type = TYPES[PARTICLE_STEAM];
            end = true;
        }
        else if (map[x-1][y]->element == PARTICLE_WOOD) {
            map[x-1][y]->element = PARTICLE_SMOKE;
            map[x-1][y]->type = TYPES[PARTICLE_SMOKE];
            if (rand() % 10 == 0) end = true;
        }
    }
    if (x < width-1) {
        if (map[x+1][y]->element == PARTICLE_WATER || map[x+1][y]->element == PARTICLE_ACID) {
            map[x+1][y]->element = PARTICLE_STEAM;
            map[x+1][y]->type = TYPES[PARTICLE_STEAM];
            end = true;
        }
        else if (map[x+1][y]->element == PARTICLE_WOOD) {
            map[x+1][y]->element = PARTICLE_SMOKE;
            map[x+1][y]->type = TYPES[PARTICLE_SMOKE];
            if (rand() % 10 == 0) end = true;
        }
    }
    if (y < height-1) {
        if (map[x][y+1]->element == PARTICLE_WATER || map[x][y+1]->element == PARTICLE_ACID) {
            map[x][y+1]->element = PARTICLE_STEAM;
            map[x][y+1]->type = TYPES[PARTICLE_STEAM];
            end = true;
        }
        else if (map[x][y+1]->element == PARTICLE_WOOD) {
            map[x][y+1]->element = PARTICLE_SMOKE;
            map[x][y+1]->type = TYPES[PARTICLE_SMOKE];
            if (rand() % 10 == 0) end = true;
        }
    }
    if (end) {
        p->element = PARTICLE_STONE;
        p->type = TYPES[PARTICLE_STONE];
        return;
    }

    if (fall_down(p, map, x, y)) return;
    if (move_side(p, map, x, y, 1)) return;
    if (flow(p, map, x, y)) return;


}

void acid_update(Particle* p, Particle*** map, int x, int y) {

    bool end = false;

    if (y < height-1) {
        if (map[x][y+1]->element == PARTICLE_WATER) {
            map[x-1][y]->element = PARTICLE_NONE;
            map[x-1][y]->type = PARTICLE_VOID;
            end = true;
        }
        else if (map[x][y+1]->element == PARTICLE_LAVA) {
            map[x][y+1]->element = PARTICLE_OBSIDIAN;
            map[x][y+1]->type = TYPES[PARTICLE_OBSIDIAN];
            end = true;
        }
        else if (map[x][y+1]->element == PARTICLE_WOOD) {
            map[x][y+1]->element = PARTICLE_NONE;
            map[x][y+1]->type = PARTICLE_VOID;
            if (rand() % 5 == 0) end = true;
        }
    }
    if (end) {
        p->element = PARTICLE_STEAM;
        p->type = TYPES[PARTICLE_STEAM];
        return;
    }

    if (fall_down(p, map, x, y)) return;
    if (flow(p, map, x, y)) return;
}

void steam_update(Particle* p, Particle*** map, int x, int y) {

    if (float_up(p, map, x, y)) return;
    if (flow(p, map, x, y)) return;
}

void smoke_update(Particle* p, Particle*** map, int x, int y) {

    if (float_up(p, map, x, y)) return;
    if (move_side(p, map, x, y, -1)) return;
    if (flow(p, map, x, y)) return;
}

// Constructs a SDL_Rect
void new_rect(SDL_Rect* rect, int x, int y, int w, int h) {
    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;
}

bool collide_rect(SDL_Rect* rect, int mx, int my) {
    
    if (rect->x <= mx && mx < rect->x + rect->w &&
        rect->y <= my && my < rect->y + rect->h) return true;
    
    return false;
}

void outline_rect(SDL_Rect* rect, SDL_Renderer* screen) {
    SDL_SetRenderDrawColor(screen, 100, 100, 120, 255);
    SDL_RenderDrawLine(screen, rect->x, rect->y, rect->x, rect->y + rect->h);
    SDL_RenderDrawLine(screen, rect->x, rect->y + rect->h, rect->x + rect->w, rect->y + rect->h);
    SDL_RenderDrawLine(screen, rect->x + rect->w, rect->y + rect->h, rect->x + rect->w, rect->y);
    SDL_RenderDrawLine(screen, rect->x + rect->w, rect->y, rect->x, rect->y);
}

// Main function
int main(int argc, char* argv[]) {

    // Seeding random
    srand(time(NULL));

    // SDL innit
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
    SCREEN_WIDTH, SCREEN_HEIGHT + MENU_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* screen = SDL_CreateRenderer(window, -1, 0);

    // Event variable
    SDL_Event event;

    // Input variables
    bool mouse_left_down = false;
    bool mouse_right_down = false;
    int mouse_pos_x = 0;
    int mouse_pos_y = 0;

    // FPS vriables
    int delay = 1000 / FPS;
    int sleep = 0;

    // Time variables
    unsigned int start_time = 0;
    unsigned int end_time = 0;
    unsigned int delta = 0;
    
    // Particle map
    Particle*** particles = malloc(sizeof(Particle**) * width);
    for (int w=0; w < width; w++) {
        particles[w] = malloc(sizeof(Particle*) * height);
    }
    for (int w=0; w < width; w++) {
        for (int h=0; h < height; h++) {
            particles[w][h] = malloc(sizeof(Particle));
            new_particle(particles[w][h], PARTICLE_VOID, PARTICLE_NONE);
        }
    }

    // Particle data
    int particle_update_delay = 0;
    int particle_type = 0;
    int draw_size = 0;

    // Different update functions for different types
    function particle_update_functions[length] = {sand_update, dirt_update, stone_update, obsidian_update, steel_update, wood_update, water_update, lava_update, acid_update, steam_update, smoke_update};

    // Utility rects
    SDL_Rect particle_rect;
    SDL_Rect preview_rect;
    SDL_Rect menu_rect;
    new_rect(&particle_rect, 0, 0, PARTICLE_SIZE, PARTICLE_SIZE);
    new_rect(&preview_rect, 0, 0, PARTICLE_SIZE, PARTICLE_SIZE);
    new_rect(&menu_rect, 0, 0, SCREEN_WIDTH, MENU_HEIGHT);

    // Buttons
    SDL_Rect button_size1;
    SDL_Rect button_size2;
    SDL_Rect button_size3;
    SDL_Rect button_particle;
    
    new_rect(&button_size1, 24, 34, 30, 30);
    new_rect(&button_size2, 64, 29, 40, 40);
    new_rect(&button_size3, 114, 24, 50, 50);
    new_rect(&button_particle, 0, 0, 40, 40);

    // Main loop
    bool running = true;
    while (running) {

        start_time = SDL_GetTicks();  // Gets loop start time

        // Get mouse position
        SDL_GetMouseState(&mouse_pos_x, &mouse_pos_y);  // Gets mouse position

        // Handles events
        SDL_PollEvent(&event);  // Gets event
        if (event.type == SDL_QUIT) {
            running = false;
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {  // Mouse down
            
            if (event.button.button == SDL_BUTTON_LEFT && !mouse_left_down) {  // Left click
                mouse_left_down = true;

                if (collide_rect(&button_size1, mouse_pos_x, mouse_pos_y)) draw_size = 0;
                else if (collide_rect(&button_size2, mouse_pos_x, mouse_pos_y)) draw_size = 1;
                else if (collide_rect(&button_size3, mouse_pos_x, mouse_pos_y)) draw_size = 2;

                preview_rect.w = PARTICLE_SIZE * (2 * draw_size + 1);
                preview_rect.h = PARTICLE_SIZE * (2 * draw_size + 1);

                int x = 200;
                int y = 30;
                for (int button=0; button < length; button++) {

                    button_particle.x = x + 40*button;
                    button_particle.y = y;
                   
                    if (collide_rect(&button_particle, mouse_pos_x, mouse_pos_y)) {
                        particle_type = button;
                        break;
                    }
                }
            }
            else if (event.button.button == SDL_BUTTON_RIGHT && !mouse_right_down) {  // Right click
                mouse_right_down = true;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONUP) {  // Mouse up
            if (event.button.button == SDL_BUTTON_LEFT && mouse_left_down) {  // Left click
                mouse_left_down = false;
            }
            else if (event.button.button == SDL_BUTTON_RIGHT && mouse_right_down) {  // Rightclick
                mouse_right_down = false;
            }
        }

        // Creates particles
        if (mouse_left_down) {

            if(mouse_pos_y >= MENU_HEIGHT) {
                int x, y;
            
                x = mouse_pos_x / PARTICLE_SIZE;
                y = (mouse_pos_y - MENU_HEIGHT) / PARTICLE_SIZE;

                for (int i = -1*draw_size; i <= draw_size; i++) {
                    for (int j = -1*draw_size; j <= draw_size; j++) {

                        int xi = x+i;
                        int yj = y+j;
                        if (xi < 0 || xi >= width || yj < 0 || yj >= height) continue;
                        if (particles[xi][yj]->type == PARTICLE_VOID) {
                            particles[xi][yj]->type = TYPES[particle_type];
                            particles[xi][yj]->element = particle_type;
                        }
                    }
                }
            }
        }

        // Destroy particles
        if (mouse_right_down) {

            if(mouse_pos_y >= MENU_HEIGHT) {
                int x, y;
            
                x = mouse_pos_x / PARTICLE_SIZE;
                y = (mouse_pos_y - MENU_HEIGHT) / PARTICLE_SIZE;

                for (int i = -1*draw_size; i <= draw_size; i++) {
                    for (int j = -1*draw_size; j <= draw_size; j++) {

                        int xi = x+i;
                        int yj = y+j;
                        if (xi < 0 || xi >= width || yj < 0 || yj >= height) continue;
                        if (particles[xi][yj]->type != PARTICLE_VOID) {
                            particles[xi][yj]->type = PARTICLE_VOID;
                            particles[xi][yj]->element = PARTICLE_NONE;
                        }
                    }
                }
            }
        }

        // Update particles every n frames
        if (particle_update_delay == PARTICLE_UPDATE_DELAY) {

            for (int w=0; w < width; w++) {
                for (int h=0; h < height; h++) {

                    Particle* p = particles[w][h];

                    if (p->type == PARTICLE_VOID || p->updated) continue;

                    p->updated = true;
                    particle_update_functions[p->element](p, particles, w, h);
                }
            }
            particle_update_delay = 0;
        }
        else particle_update_delay++;

        // Enables alpha
        SDL_SetRenderDrawBlendMode(screen, SDL_BLENDMODE_BLEND);
        
        // Clears screen
        SDL_RenderClear(screen);

        // Display particles
        int r, g, b;
        for (int w=0; w < width; w++) {
            for (int h=0; h < height; h++) {

                Particle* p = particles[w][h];

                if (p->type == PARTICLE_VOID) continue;

                p->updated = false;;

                particle_rect.x = w * PARTICLE_SIZE;
                particle_rect.y = h * PARTICLE_SIZE + MENU_HEIGHT;

                r = COLORS[p->element][0];
                g = COLORS[p->element][1];
                b = COLORS[p->element][2];

                SDL_SetRenderDrawColor(screen, r, g, b, 255);
                SDL_RenderFillRect(screen, &particle_rect);
            }
        }

        // Display preview
        preview_rect.x = mouse_pos_x / PARTICLE_SIZE * PARTICLE_SIZE - PARTICLE_SIZE*draw_size;
        preview_rect.y = mouse_pos_y / PARTICLE_SIZE * PARTICLE_SIZE - PARTICLE_SIZE*draw_size;

        r = COLORS[particle_type][0];
        g = COLORS[particle_type][1];
        b = COLORS[particle_type][2];

        SDL_SetRenderDrawColor(screen, r, g, b, 100);
        SDL_RenderFillRect(screen, &preview_rect);

        // Displays menu
        SDL_SetRenderDrawColor(screen, 240, 240, 250, 255);
        SDL_RenderFillRect(screen, &menu_rect);

        SDL_SetRenderDrawColor(screen, 200, 200, 210, 255);
        SDL_RenderFillRect(screen, &button_size1);
        SDL_RenderFillRect(screen, &button_size2);
        SDL_RenderFillRect(screen, &button_size3);

        if (draw_size == 0) outline_rect(&button_size1, screen);
        if (draw_size == 1) outline_rect(&button_size2, screen);
        if (draw_size == 2) outline_rect(&button_size3, screen);

        int x = 200;
        int y = 30;
        for (int button=0; button < length; button++) {

            button_particle.x = x + 40*button;
            button_particle.y = y;

            if (particle_type == button) {
                button_particle.x = x + 40*button + 2;
                button_particle.y = y + 2;
                button_particle.w = 36;
                button_particle.h = 36;
            }

            r = COLORS[button][0];
            g = COLORS[button][1];
            b = COLORS[button][2];

            SDL_SetRenderDrawColor(screen, r, g, b, 255);
            SDL_RenderFillRect(screen, &button_particle);

            button_particle.w = 40;
            button_particle.h = 40;
        }

        // Fills background
        SDL_SetRenderDrawColor(screen, 20, 20, 30, 255);
        SDL_RenderPresent(screen);

        end_time = SDL_GetTicks();  // Gets loop end time
        delta = end_time - start_time;  // Calculates loop execution time

        sleep = delay - delta;  // Calculates sleep time
        if (sleep < 0) sleep = 0;  // Prevents delay smaller then 0

        SDL_Delay(sleep);  // Starts delay
    }

    // Deallocates particles
    /*for (int w=0; w < width; w++) {
        for (int h=0; w < height; h++) {
            free(particles[w][h]);
        }
    }*/
    for (int w=0; w < width; w++) {
        free(particles[w]);
    }
    free(particles);

    SDL_Quit();
    return 0;
}
