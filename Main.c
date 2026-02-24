#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "header.h"

/* Declared in Display_board.c */
void sdl_init(int board_size);
void sdl_quit(void);
int  px_to_col(int px);
int  py_to_line(int py);
int  px_to_wall_gap(int px, int py, int *col, int *line);
void draw_preview(int hover_type, int col, int line, int size, int is_black);

int turn = 0;

/* ── build a two-char move string from col + line, e.g. col=1,line=3 → "A3" */
static void make_move_str(char out[4], int col, int line) {
    out[0] = 'A' + col - 1;
    out[1] = '0' + line;
    out[2] = '\n';
    out[3] = '\0';
}

/* ── build a wall string from col + line + type, e.g. "A3h\n" ───────────── */
static void make_wall_str(char out[5], int col, int line, char type) {
    out[0] = 'A' + col - 1;
    out[1] = '0' + line;
    out[2] = type;
    out[3] = '\n';
    out[4] = '\0';
}

int main(void) {
    struct board  table;
    struct player *white, *black;
    struct list   *list_mem = NULL;
    int  walls, h = 0, v = 0, tr;

    /* ── setup (still read size and walls from terminal once) ───────────── */
    printf("boardsize : ");
    scanf("%d", &table.size);
    printf("walls     : ");
    scanf("%d", &walls);
    table.walls_b = walls;
    table.walls_w = walls;


    white = malloc(sizeof *white);
    black = malloc(sizeof *black);

    /* VLAs for wall storage — same as before */
    int wall_ho[walls * 2][2];
    int wall_ve[walls * 4][2];
    memset(wall_ho, 0, sizeof(wall_ho));
    memset(wall_ve, 0, sizeof(wall_ve));

    /* ── open the SDL window ─────────────────────────────────────────────── */
    sdl_init(table.size);

    /* ── initial pawn positions ──────────────────────────────────────────── */
    white->line = 1;
    white->row  = table.size / 2 + 1;
    black->line = table.size;
    black->row  = table.size / 2 + 1;
    insert_at_end(&list_mem, black->row, black->line);
    insert_at_end(&list_mem, white->row, white->line);
    turn = 1; /* skip turn-0 init block — already done above */

    /* wall_mode removed — wall type is always auto-detected from click position */

    /* hover state — updated on every mouse motion */
    int hover_type = -1;  /* -1=nothing, 0=pawn, 1=horiz wall, 2=vert wall */
    int hover_col  = 0;
    int hover_line = 0;

    /* Draw the initial board */
    display_board(table.size, table.walls_w, table.walls_b,
                  white->line, white->row, black->line, black->row,
                  walls, wall_ho, wall_ve);
    draw_preview(-1, 0, 0, table.size, 0);

    /* ── main SDL event loop ─────────────────────────────────────────────── */
    SDL_Event ev;
    int running = 1;

    while (running) {

        /* check win condition */
        if (white->line == table.size) {
            printf("The winner is WHITE!\n");
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
                                     "Game Over", "White wins!", NULL);
            running = 0;
            break;
        }
        if (black->line == 1) {
            printf("The winner is BLACK!\n");
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
                                     "Game Over", "Black wins!", NULL);
            running = 0;
            break;
        }

        /* wait for the next event */
        if (SDL_WaitEvent(&ev) == 0) continue;

        /* ── mouse motion → update hover preview ────────────────────────── */
        if (ev.type == SDL_MOUSEMOTION) {
            /* Drain all queued motion events and keep only the last one.
               This prevents the preview from lagging behind the mouse when
               many motion events pile up faster than we can draw.           */
            SDL_Event next;
            while (SDL_PeepEvents(&next, 1, SDL_GETEVENT,
                                  SDL_MOUSEMOTION, SDL_MOUSEMOTION) > 0)
                ev = next;

            int mx = ev.motion.x;
            int my = ev.motion.y;
            int wcol, wline;
            int gap = px_to_wall_gap(mx, my, &wcol, &wline);
            int is_black = (turn % 2 == 1);

            /* auto-detect: gap → wall preview, cell → pawn preview */
            if (gap == 1)      { hover_type = 1; hover_col = wcol; hover_line = wline; }
            else if (gap == 2) { hover_type = 2; hover_col = wcol; hover_line = wline; }
            else {
                int c = px_to_col(mx);
                int l = py_to_line(my);
                if (c != 0 && l != 0) {
                    int op_row  = (turn % 2 == 1) ? white->row  : black->row;
                    int op_line = (turn % 2 == 1) ? white->line : black->line;
                    int cur_row  = (turn % 2 == 1) ? black->row  : white->row;
                    int cur_line = (turn % 2 == 1) ? black->line : white->line;
                    if (c == op_row && l == op_line) {
                        hover_col  = cur_row  + (op_row  - cur_row)  * 2;
                        hover_line = cur_line + (op_line - cur_line) * 2;
                    } else {
                        hover_col  = c;
                        hover_line = l;
                    }
                    hover_type = 0;
                }
                else { hover_type = -1; }
            }

            /* redraw board + preview every motion event */
            display_board(table.size, table.walls_w, table.walls_b,
                          white->line, white->row,
                          black->line, black->row,
                          walls, wall_ho, wall_ve);
            draw_preview(hover_type, hover_col, hover_line, table.size, is_black);
            continue;
        }

        /* ── keyboard ────────────────────────────────────────────────────── */
        if (ev.type == SDL_QUIT) {
            running = 0;
            break;
        }

        if (ev.type == SDL_KEYDOWN) {
            switch (ev.key.keysym.sym) {

                case SDLK_q:
                    running = 0;
                    break;

                /* [G] — let AI play the current turn */
                case SDLK_g: {
                    if (turn % 2 == 1) {
                        ai_generate(&(black->row), &(black->line),
                                    &(white->row), &(white->line),
                                    list_mem, wall_ho,
                                    2 * walls - table.walls_b, turn, table.size);
                        insert_at_end(&list_mem, black->row, black->line);
                    } else {
                        ai_generate(&(white->row), &(white->line),
                                    &(black->row), &(black->line),
                                    list_mem, wall_ho,
                                    2 * walls - table.walls_w, turn, table.size);
                        insert_at_end(&list_mem, white->row, white->line);
                    }
                    turn++;
                    display_board(table.size, table.walls_w, table.walls_b,
                                  white->line, white->row,
                                  black->line, black->row,
                                  walls, wall_ho, wall_ve);
                    draw_preview(-1, 0, 0, table.size, 0);
                    break;
                }

                default: break;
            }
            continue;
        }

        /* ── mouse click ─────────────────────────────────────────────────── */
        if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
            int mx = ev.button.x;
            int my = ev.button.y;

            int wcol, wline;
            int gap_type = px_to_wall_gap(mx, my, &wcol, &wline);

            if (gap_type != 0) {
                /* ── wall placement — type comes directly from click position */
                int *walls_left = (turn % 2 == 1) ? &table.walls_b : &table.walls_w;
                if (*walls_left == 0) {
                    printf("No walls left for this player.\n");
                    continue;
                }
                char wall_str[5];
                char type = (gap_type == 1) ? 'h' : 'v';
                make_wall_str(wall_str, wcol, wline, type);

                if (gap_type == 1) {
                    tr = playwall(wall_str, wall_ho, &h, 1);
                    if (tr == 0) {
                        (*walls_left)--;
                        sorting(wall_ho, h);
                    }
                } else {
                    tr = playwall(wall_str, wall_ve, &v, 0);
                    if (tr == 0) {
                        (*walls_left)--;
                        sorting(wall_ve, v);
                    }
                }
                if (tr != 0) { printf("Invalid wall placement.\n"); continue; }
                turn++;

            } else {
                /* ── pawn move — clicked inside a cell */
                int col  = px_to_col(mx);
                int line = py_to_line(my);
                if (col == 0 || line == 0) continue;

                char move_str[4];
                make_move_str(move_str, col, line);

                if (turn % 2 == 1) {
                    tr = playmove(move_str,
                                  &(black->row), &(black->line),
                                  white->row, white->line,
                                  wall_ho, wall_ve,
                                  2 * walls - table.walls_b - table.walls_w);
                    if (tr == 0) { insert_at_end(&list_mem, black->row, black->line); turn++; }
                } else {
                    tr = playmove(move_str,
                                  &(white->row), &(white->line),
                                  black->row, black->line,
                                  wall_ho, wall_ve,
                                  2 * walls - table.walls_b - table.walls_w);
                    if (tr == 0) { insert_at_end(&list_mem, white->row, white->line); turn++; }
                }
            }

            /* redraw after every processed click */
            display_board(table.size, table.walls_w, table.walls_b,
                          white->line, white->row,
                          black->line, black->row,
                          walls, wall_ho, wall_ve);
            draw_preview(-1, 0, 0, table.size, 0);
        }
    }

    /* ── cleanup ─────────────────────────────────────────────────────────── */
    sdl_quit();
    free(white);
    free(black);
    while (list_mem != NULL) {
        struct list *tmp = list_mem->next;
        free(list_mem);
        list_mem = tmp;
    }
    return 0;
}