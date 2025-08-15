#ifndef SNAKE_H
#define SNAKE_H

#include "kernel.h"

// Direction enumeration
typedef enum {
    DIRECTION_UP = 0,
    DIRECTION_DOWN = 1,
    DIRECTION_LEFT = 2,
    DIRECTION_RIGHT = 3
} direction_t;

// Game state structure
typedef struct {
    int score;
    int game_over;
    int snake_length;
    direction_t direction;
    direction_t next_direction;
} snake_game_t;

// Function declarations
void snake_init(void);
void snake_update(void);
void snake_draw(void);
void snake_place_food(void);
int snake_check_collision_with_body(int x, int y);
void handle_uart_input(char c);

#endif