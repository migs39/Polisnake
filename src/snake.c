#include "snake.h"
#include "graphics.h"
#include "framebuffer.h"
#include "gpio.h"
#include "uart.h"
#include "timer.h"

// Game constants
#define GRID_WIDTH      40
#define GRID_HEIGHT     30
#define CELL_SIZE       16
#define GRID_OFFSET_X   80
#define GRID_OFFSET_Y   80
#define MAX_SNAKE_LENGTH (GRID_WIDTH * GRID_HEIGHT)

// Snake structure
typedef struct {
    int x, y;
} point_t;

// Game state
static snake_game_t game;
static point_t snake_body[MAX_SNAKE_LENGTH];
static point_t food;
static int last_input_time = 0;

// Input debouncing
#define INPUT_DEBOUNCE_TIME 100 // ms

void snake_init(void) {
    // Initialize game state
    game.score = 0;
    game.game_over = 0;
    game.snake_length = 3;
    game.direction = DIRECTION_RIGHT;
    game.next_direction = DIRECTION_RIGHT;
    
    // Initialize snake in the middle of the grid
    snake_body[0].x = GRID_WIDTH / 2;      // Head
    snake_body[0].y = GRID_HEIGHT / 2;
    snake_body[1].x = GRID_WIDTH / 2 - 1;  // Body
    snake_body[1].y = GRID_HEIGHT / 2;
    snake_body[2].x = GRID_WIDTH / 2 - 2;  // Tail
    snake_body[2].y = GRID_HEIGHT / 2;
    
    // Place initial food
    snake_place_food();
    
    uart_puts("Snake game initialized!\n");
    uart_puts("Score: 0\n");
}

void snake_place_food(void) {
    int attempts = 0;
    do {
        food.x = timer_get_ticks() % GRID_WIDTH;
        food.y = (timer_get_ticks() / 7) % GRID_HEIGHT;
        attempts++;
        
        // Prevent infinite loop
        if (attempts > 100) {
            food.x = 0;
            food.y = 0;
            break;
        }
    } while (snake_check_collision_with_body(food.x, food.y));
}

int snake_check_collision_with_body(int x, int y) {
    for (int i = 0; i < game.snake_length; i++) {
        if (snake_body[i].x == x && snake_body[i].y == y) {
            return 1;
        }
    }
    return 0;
}

void snake_update(void) {
    if (game.game_over) return;
    
    // Handle input (GPIO buttons)
    uint32_t current_time = timer_get_system_timer() * 10; // Convert to ms
    if (current_time - last_input_time > INPUT_DEBOUNCE_TIME) {
        if (gpio_read_button_up() && game.direction != DIRECTION_DOWN) {
            game.next_direction = DIRECTION_UP;
            last_input_time = current_time;
        }
        else if (gpio_read_button_down() && game.direction != DIRECTION_UP) {
            game.next_direction = DIRECTION_DOWN;
            last_input_time = current_time;
        }
        else if (gpio_read_button_left() && game.direction != DIRECTION_RIGHT) {
            game.next_direction = DIRECTION_LEFT;
            last_input_time = current_time;
        }
        else if (gpio_read_button_right() && game.direction != DIRECTION_LEFT) {
            game.next_direction = DIRECTION_RIGHT;
            last_input_time = current_time;
        }
    }
    
    // Update direction
    game.direction = game.next_direction;
    
    // Move snake body (start from tail)
    for (int i = game.snake_length - 1; i > 0; i--) {
        snake_body[i] = snake_body[i - 1];
    }
    
    // Move head
    switch (game.direction) {
        case DIRECTION_UP:
            snake_body[0].y--;
            break;
        case DIRECTION_DOWN:
            snake_body[0].y++;
            break;
        case DIRECTION_LEFT:
            snake_body[0].x--;
            break;
        case DIRECTION_RIGHT:
            snake_body[0].x++;
            break;
    }
    
    // Check wall collision
    if (snake_body[0].x < 0 || snake_body[0].x >= GRID_WIDTH ||
        snake_body[0].y < 0 || snake_body[0].y >= GRID_HEIGHT) {
        game.game_over = 1;
        uart_puts("Game Over! Hit wall.\n");
        return;
    }
    
    // Check self collision
    for (int i = 1; i < game.snake_length; i++) {
        if (snake_body[0].x == snake_body[i].x && snake_body[0].y == snake_body[i].y) {
            game.game_over = 1;
            uart_puts("Game Over! Hit self.\n");
            return;
        }
    }
    
    // Check food collision
    if (snake_body[0].x == food.x && snake_body[0].y == food.y) {
        game.score += 10;
        game.snake_length++;
        
        uart_puts("Food eaten! Score: ");
        uart_dec(game.score);
        uart_puts("\n");
        
        // Place new food
        snake_place_food();
        
        // Flash LED
        gpio_led_on();
        timer_sleep(50);
        gpio_led_off();
    }
}

void snake_draw(void) {
    // Clear screen
    graphics_clear_screen(COLOR_BLACK);
    
    // Draw game area border
    graphics_draw_rect_outline(
        GRID_OFFSET_X - 2, 
        GRID_OFFSET_Y - 2, 
        GRID_WIDTH * CELL_SIZE + 4, 
        GRID_HEIGHT * CELL_SIZE + 4, 
        COLOR_WHITE
    );
    
    // Draw snake
    for (int i = 0; i < game.snake_length; i++) {
        uint32_t color = (i == 0) ? COLOR_GREEN : COLOR_DARK_GRAY; // Head vs body
        
        int pixel_x = GRID_OFFSET_X + snake_body[i].x * CELL_SIZE;
        int pixel_y = GRID_OFFSET_Y + snake_body[i].y * CELL_SIZE;
        
        graphics_draw_rect(pixel_x, pixel_y, CELL_SIZE - 1, CELL_SIZE - 1, color);
    }
    
    // Draw food
    int food_pixel_x = GRID_OFFSET_X + food.x * CELL_SIZE;
    int food_pixel_y = GRID_OFFSET_Y + food.y * CELL_SIZE;
    graphics_draw_rect(food_pixel_x, food_pixel_y, CELL_SIZE - 1, CELL_SIZE - 1, COLOR_RED);
    
    // Draw score
    graphics_draw_text("SCORE:", 10, 10, COLOR_WHITE);
    
    // Convert score to string and display
    char score_str[16];
    int score_val = game.score;
    int pos = 0;
    
    if (score_val == 0) {
        score_str[pos++] = '0';
    } else {
        char temp[16];
        int temp_pos = 0;
        
        while (score_val > 0) {
            temp[temp_pos++] = '0' + (score_val % 10);
            score_val /= 10;
        }
        
        // Reverse string
        for (int i = temp_pos - 1; i >= 0; i--) {
            score_str[pos++] = temp[i];
        }
    }
    score_str[pos] = '\0';
    
    graphics_draw_text(score_str, 70, 10, COLOR_YELLOW);
    
    // Draw controls info
    graphics_draw_text("GPIO: 2=UP 3=DOWN 4=LEFT 17=RIGHT", 10, 30, COLOR_CYAN);
    graphics_draw_text("UART: W=UP S=DOWN A=LEFT D=RIGHT", 10, 50, COLOR_CYAN);
    
    // Draw game over message
    if (game.game_over) {
        graphics_draw_text("GAME OVER!", GRID_OFFSET_X + 100, GRID_OFFSET_Y + 200, COLOR_RED);
        graphics_draw_text("Reset to play again", GRID_OFFSET_X + 50, GRID_OFFSET_Y + 220, COLOR_WHITE);
    }
}

// Override the weak symbol from interrupts.c
void handle_uart_input(char c) {
    // Convert to lowercase
    if (c >= 'A' && c <= 'Z') {
        c = c + ('a' - 'A');
    }
    
    // Handle snake controls
    uint32_t current_time = timer_get_system_timer() * 10; // Convert to ms
    if (current_time - last_input_time > INPUT_DEBOUNCE_TIME) {
        switch (c) {
            case 'w':
                if (game.direction != DIRECTION_DOWN) {
                    game.next_direction = DIRECTION_UP;
                    last_input_time = current_time;
                    uart_puts("UP\n");
                }
                break;
            case 's':
                if (game.direction != DIRECTION_UP) {
                    game.next_direction = DIRECTION_DOWN;
                    last_input_time = current_time;
                    uart_puts("DOWN\n");
                }
                break;
            case 'a':
                if (game.direction != DIRECTION_RIGHT) {
                    game.next_direction = DIRECTION_LEFT;
                    last_input_time = current_time;
                    uart_puts("LEFT\n");
                }
                break;
            case 'd':
                if (game.direction != DIRECTION_LEFT) {
                    game.next_direction = DIRECTION_RIGHT;
                    last_input_time = current_time;
                    uart_puts("RIGHT\n");
                }
                break;
            case 'r':
                if (game.game_over) {
                    snake_init(); // Restart game
                    uart_puts("Game restarted!\n");
                }
                break;
            default:
                // Echo other characters
                uart_putc(c);
                break;
        }
    }
}