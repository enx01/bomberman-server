#include "headers/game.h"
#include "headers/player.h"
#include "headers/bomb.h"
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>


game *setup_game() {
    game *res = malloc(sizeof(game));
    res->h = GAME_HEIGHT;
    res->w = GAME_WIDTH;

    res->ongoing = 0;


    res->b = malloc(sizeof(board));
    res->b->h = res->h - 2 - 1; // 2 rows reserved for border, 1 row for line 15
    res->b->w = res->w - 2; // 2 columns reserved for border 15
    

    int dim = res->b->h * res->b->w;

    res->b->grid = calloc(dim, sizeof(char));

   
    char *grid_ffa = calloc(dim, sizeof(char));

    for (int x = 0; x < res->b->w ; x++) {
        for (int y = 0; y < res->b->h; y++) {
            if (    y != 0 && 
                    y % 2 == 0 &&
                    x % 2 == 0 &&
                    y != (res->b->h-1)
                    ) {
            grid_ffa[y*res->b->w + x] = 1;
            }


        }
        
    }

    for (int i = 0; i < dim; ++i) {
        res->b->grid[i] = grid_ffa[i];
    }

    //memcpy(res->b->grid, grid_ffa, dim);
    free(grid_ffa);
    return res;
}

// void run_game(game *g, line *l) {
//     int diff[1000] = {0}; // Example size, should be large enough to hold all changes
//     int whole[1000] = {0}; // Example size, should be large enough to hold entire board
//     update_game(g, diff, 0, whole); // Should only be done server-side
//     render_game(g, l); // Should only be done client-side
// }

int get_grid(board* b, int x, int y) {
    return b->grid[y*b->w + x];
}

void set_grid(board* b, int x, int y, int v) {
    b->grid[y*b->w + x] = v;
}

void render_game(game *g, line *l) {
    // Update grid
    int x,y;
    for (y = 0; y < g->b->h; y++) {
        for (x = 0; x < g->b->w; x++) {
            int i = get_grid(g->b,x,y);
            switch (i) {
                case 0: // Free Space
                    mvaddch(y+1,x+1,' ');
                    break;
                case 1: // Wall
                    mvaddch(y+1,x+1,'#');
                    break;
                case 2: // Player -> DWall
                    mvaddch(y+1,x+1, '~');
                    break;
                case 3: // Explosion Ray -> Bomb
                    mvaddch(y+1,x+1, 'o');
                    break;
                case 4: // Bomb timer '3' -> Explosion Ray
                    mvaddch(y+1,x+1, 'x');
                    break;
                case 5: // Bomb timer '2' -> Player 1
                    attron(COLOR_PAIR(1));
                    mvaddch(y+1,x+1, 'o');
                    attroff(COLOR_PAIR(1));
                    break;
                case 6: // Bomb timer '1' -> Player 2
                    attron(COLOR_PAIR(2));
                    mvaddch(y+1,x+1, 'o');
                    attroff(COLOR_PAIR(2));
                    break;
                case 7: // Player 3
                    attron(COLOR_PAIR(3));
                    mvaddch(y+1,x+1, 'o');
                    attroff(COLOR_PAIR(3));
                    break;
                case 8: // Player 4
                    attron(COLOR_PAIR(4));
                    mvaddch(y+1,x+1, 'o');
                    attroff(COLOR_PAIR(4));
                    break;
                default:
                    mvaddch(y+1,x+1, i + '?');
                    break;
            }
        }
    }
    
    // Borders
    for (x = 0; x < g->b->w+2; x++) {
        mvaddch(0, x, '-');
        mvaddch(g->b->h+1, x, '-');
    }
    for (y = 0; y < g->b->h+2; y++) {
        mvaddch(y, 0, '|');
        mvaddch(y, g->b->w+1, '|');
    }
    mvaddch(0, 0,'o');
    mvaddch(0, g->b->w + 1,'o');
    mvaddch(g->b->h + 1, 0,'o');
    mvaddch(g->b->h + 1, g->b->w + 1,'o');
    
    
    refresh(); // Apply the changes to the terminal
}


void update_game(game *g, uint8_t *diff, uint8_t *nb_diff, uint8_t *whole) {
    int diff_count = 0; // Counter for the number of updates

    for (int i = 0; i < g->player_count; i++) {
        if (g->players_list[i]->state == DEAD) {
            goto bomb_placing;
        } else {
            // Players collisions
            if (g->players_list[i]->next_y >= 0 &&
                g->players_list[i]->next_y < g->b->h &&
                g->players_list[i]->next_x >= 0 &&
                g->players_list[i]->next_x < g->b->w &&
                get_grid(g->b, g->players_list[i]->next_x, g->players_list[i]->next_y) == 0) {
                
                set_grid(g->b, g->players_list[i]->pos_x, g->players_list[i]->pos_y, 0);
                update_player(g->players_list[i]);
                set_grid(g->b, g->players_list[i]->pos_x, g->players_list[i]->pos_y, (5 + i));
                diff[diff_count] = g->players_list[i]->pos_x;
                diff[diff_count+1] = g->players_list[i]->pos_y;
                diff[diff_count+2] = 5+i;
                diff_count += 3;
            } else if (get_grid(g->b, g->players_list[i]->next_x, g->players_list[i]->next_y) == 4) {
                set_grid(g->b, g->players_list[i]->pos_x, g->players_list[i]->pos_y, 0);
                g->players_list[i]->state = DEAD;
                diff[diff_count] = g->players_list[i]->pos_x;
                diff[diff_count+1] = g->players_list[i]->pos_y;
                diff[diff_count+2] = 0;
                diff_count += 3;
            } else {
                reset_next(g->players_list[i]);
            }

            // Bomb placing
            if (g->players_list[i]->is_placing_bomb) {
                g->players_list[i]->is_placing_bomb = 0;
                g->players_list[i]->has_bomb = 0;

                bomb *b = create_bomb(g->players_list[i]->pos_x, g->players_list[i]->pos_y, g->players_list[i]->id);
                g->bomb_list[g->players_list[i]->id] = b;
                set_grid(g->b, g->players_list[i]->pos_x, g->players_list[i]->pos_y, 3);
                diff[diff_count] = g->players_list[i]->pos_x;
                diff[diff_count+1] = g->players_list[i]->pos_y;
                diff[diff_count+2] = 3;
                diff_count += 3;
            }
        }

        bomb_placing:
        // Updating bombs
        if (!g->players_list[i]->has_bomb) {
            bomb *temp = g->bomb_list[g->players_list[i]->id];
            update_bomb(temp);

            switch (temp->state) {
                case BOMB:
                    set_grid(g->b, temp->pos_x, temp->pos_y, 3);
                    diff[diff_count] = temp->pos_x;
                    diff[diff_count+1] = temp->pos_y;
                    diff[diff_count+2] = 3;
                    diff_count += 3;
                    break;
                case EXPLOSION:
                    set_grid(g->b, temp->pos_x, temp->pos_y, 4);
                    diff[diff_count] = temp->pos_x;
                    diff[diff_count+1] = temp->pos_y;
                    diff[diff_count+2] = 4;
                    diff_count += 3;

                    if (get_grid(g->b, temp->pos_x + 1, temp->pos_y) == 0) {
                        set_grid(g->b, temp->pos_x + 1, temp->pos_y, 4);
                        diff[diff_count] = temp->pos_x + 1;
                        diff[diff_count+1] = temp->pos_y;
                        diff[diff_count+2] = 4;
                        diff_count += 3;
                    }

                    if (get_grid(g->b, temp->pos_x - 1, temp->pos_y) == 0) {
                        set_grid(g->b, temp->pos_x - 1, temp->pos_y, 4);
                        diff[diff_count] = temp->pos_x - 1;
                        diff[diff_count+1] = temp->pos_y;
                        diff[diff_count+2] = 4;
                        diff_count += 3;
                    }

                    if (get_grid(g->b, temp->pos_x, temp->pos_y + 1) == 0) {
                        set_grid(g->b, temp->pos_x, temp->pos_y + 1, 4);
                        diff[diff_count++] = (((temp->pos_y + 1) * g->b->w) + temp->pos_x);
                        diff[diff_count] = temp->pos_x;
                        diff[diff_count+1] = temp->pos_y + 1;
                        diff[diff_count+2] = 4;
                        diff_count += 3;
                    }

                    if (get_grid(g->b, temp->pos_x, temp->pos_y - 1) == 0) {
                        set_grid(g->b, temp->pos_x, temp->pos_y - 1, 4);
                        diff[diff_count++] = (((temp->pos_y - 1) * g->b->w) + temp->pos_x);
                        diff[diff_count++] = (((temp->pos_y + 1) * g->b->w) + temp->pos_x);
                        diff[diff_count] = temp->pos_x;
                        diff[diff_count+1] = temp->pos_y - 1;
                        diff[diff_count+2] = 4;
                        diff_count += 3;
                    }
                    // TODO : Add remaining death zones
                    break;
                case REMOVE:
                    g->players_list[i]->has_bomb = 1;
                    set_grid(g->b, temp->pos_x, temp->pos_y, 0);
                    diff[diff_count] = temp->pos_x;
                    diff[diff_count+1] = temp->pos_y;
                    diff[diff_count+2] = 0;
                    diff_count += 3;

                    if (get_grid(g->b, temp->pos_x + 1, temp->pos_y) == 4) {
                        set_grid(g->b, temp->pos_x + 1, temp->pos_y, 0);
                        diff[diff_count] = temp->pos_x + 1;
                        diff[diff_count+1] = temp->pos_y;
                        diff[diff_count+2] = 0;
                        diff_count += 3;
                    }

                    if (get_grid(g->b, temp->pos_x - 1, temp->pos_y) == 4) {
                        set_grid(g->b, temp->pos_x - 1, temp->pos_y, 0);
                        diff[diff_count] = temp->pos_x - 1;
                        diff[diff_count+1] = temp->pos_y;
                        diff[diff_count+2] = 0;
                        diff_count += 3;
                    }

                    if (get_grid(g->b, temp->pos_x, temp->pos_y + 1) == 4) {
                        set_grid(g->b, temp->pos_x, temp->pos_y + 1, 0);
                        diff[diff_count] = temp->pos_x;
                        diff[diff_count+1] = temp->pos_y + 1;
                        diff[diff_count+2] = 0;
                        diff_count += 3;
                    }

                    if (get_grid(g->b, temp->pos_x, temp->pos_y - 1) == 4) {
                        set_grid(g->b, temp->pos_x, temp->pos_y - 1, 0);
                        diff[diff_count] = temp->pos_x;
                        diff[diff_count+1] = temp->pos_y - 1;
                        diff[diff_count+2] = 0;
                        diff_count += 3;
                    }

                    free(temp);
                    break;
                default:
                    break;
            }
        }
    }

    int z = 0;
    for (int y = 0; y < g->b->h; y++) {
        for (int x = 0; x < g->b->w; x++) {
            int i = get_grid(g->b, x, y);
            whole[z] = i;
            z++;
        }
    }

    // Add a sentinel value at the end of the diff array to indicate the end of updates
    diff[diff_count] = -1;
    *nb_diff = diff_count/3;
}




int game_add_player(game *g, player *p) {
    if (g->player_count >= 4) {
        return 0;
    }
    else {
        g->players_list[g->player_count] = p;
        g->player_count++;
        return 1;
   }
}

void free_game(game *g) {
    free(g->b->grid);
    free(g->b);
    free(g);
}
