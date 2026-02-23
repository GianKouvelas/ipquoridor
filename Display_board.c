#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <math.h>
#include "header.h"

/* ── layout constants ───────────────────────────────────────────────────── */
#define CELL        70          /* pixels per board cell                     */
#define WALL_THICK  10          /* wall bar thickness in pixels              */
#define MARGIN      50          /* board offset from window edge             */
#define PANEL_W     220         /* right-side info panel width               */

/* ── module-level SDL state (init once, reused every draw) ─────────────── */
static SDL_Window   *g_win     = NULL;
static SDL_Renderer *g_ren     = NULL;
static TTF_Font     *g_font    = NULL;
static TTF_Font     *g_font_big= NULL;
static int           g_size    = 0;

/* ── helpers ────────────────────────────────────────────────────────────── */

static int col_to_px(int col) {
    return MARGIN + (col - 1) * CELL + CELL / 2;
}
static int line_to_py(int line, int size) {
    return MARGIN + (size - line) * CELL + CELL / 2;
}

static void draw_text(const char *text, int x, int y,
                      Uint8 red, Uint8 g, Uint8 b) {
    if (!g_font) return;
    SDL_Color col = {red, g, b, 255};
    SDL_Surface *surf = TTF_RenderText_Blended(g_font, text, col);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(g_ren, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(g_ren, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

static void draw_text_big(const char *text, int x, int y,
                          Uint8 red, Uint8 g, Uint8 b) {
    if (!g_font_big) return;
    SDL_Color col = {red, g, b, 255};
    SDL_Surface *surf = TTF_RenderText_Blended(g_font_big, text, col);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(g_ren, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(g_ren, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

static void draw_filled_circle(int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)sqrt((double)(radius * radius - dy * dy));
        SDL_RenderDrawLine(g_ren, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

/* ── public: initialise SDL window (call once from main) ───────────────── */
void sdl_init(int board_size) {
    g_size = board_size;
    int win_w = MARGIN * 2 + board_size * CELL + PANEL_W;
    int win_h = MARGIN * 2 + board_size * CELL + 30;

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    g_win = SDL_CreateWindow("Quoridor",
                             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             win_w, win_h, 0);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawBlendMode(g_ren, SDL_BLENDMODE_BLEND);

    /* Try common font paths — adjust if not found on your system */
    g_font     = TTF_OpenFont(
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14);
    g_font_big = TTF_OpenFont(
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 18);
}

/* ── public: shutdown SDL (call once at exit) ───────────────────────────── */
void sdl_quit(void) {
    if (g_font)     TTF_CloseFont(g_font);
    if (g_font_big) TTF_CloseFont(g_font_big);
    if (g_ren)      SDL_DestroyRenderer(g_ren);
    if (g_win)      SDL_DestroyWindow(g_win);
    TTF_Quit();
    SDL_Quit();
}

/* ── public: main draw function — replaces the old printf display_board ─── */
void display_board(int size, int walls_w, int walls_b,
                   int linew, int roww, int lineb, int rowb,
                   int walls, int wall_ho[][2], int wall_ve[][2]) {

    /* background */
    SDL_SetRenderDrawColor(g_ren, 0x1a, 0x1a, 0x2e, 0xff);
    SDL_RenderClear(g_ren);

    /* ── board cells ────────────────────────────────────────────────────── */
    for (int col = 1; col <= size; col++) {
        for (int line = 1; line <= size; line++) {
            int px = MARGIN + (col - 1) * CELL;
            int py = MARGIN + (size - line) * CELL;
            SDL_Rect cell = {px + 1, py + 1, CELL - 2, CELL - 2};
            SDL_SetRenderDrawColor(g_ren, 0xf0, 0xe6, 0xd3, 0xff);
            SDL_RenderFillRect(g_ren, &cell);
            SDL_SetRenderDrawColor(g_ren, 0x88, 0x88, 0x99, 0xff);
            SDL_RenderDrawRect(g_ren, &cell);
        }
    }

    /* ── column labels A B C … ──────────────────────────────────────────── */
    char label[3];
    for (int col = 1; col <= size; col++) {
        label[0] = 'A' + col - 1;
        label[1] = '\0';
        int px = MARGIN + (col - 1) * CELL + CELL / 2 - 5;
        draw_text(label, px, MARGIN + size * CELL + 6, 180, 180, 180);
        draw_text(label, px, MARGIN - 22,              180, 180, 180);
    }
    /* row number labels 1 2 3 … */
    char numstr[4];
    for (int line = 1; line <= size; line++) {
        snprintf(numstr, sizeof(numstr), "%d", line);
        int py = MARGIN + (size - line) * CELL + CELL / 2 - 8;
        draw_text(numstr, MARGIN - 28, py, 180, 180, 180);
        draw_text(numstr, MARGIN + size * CELL + 6, py, 180, 180, 180);
    }

    /* ── horizontal walls (red bars) ────────────────────────────────────── */
    /* wall_ho[i] = {col, line}: bar sits between row (line-1) and row line,
       spanning columns col and col+1.                                       */
    for (int i = 0; i < walls * 2; i++) {
        if (wall_ho[i][0] == 0 && wall_ho[i][1] == 0) continue;
        int col  = wall_ho[i][0];
        int line = wall_ho[i][1];
        int px = MARGIN + (col - 1) * CELL;
        int py = MARGIN + (size - line) * CELL + CELL - WALL_THICK / 2;
        SDL_Rect wall = {px, py, CELL * 2, WALL_THICK};
        SDL_SetRenderDrawColor(g_ren, 0xe7, 0x4c, 0x3c, 255);
        SDL_RenderFillRect(g_ren, &wall);
    }

    /* ── vertical walls (blue bars) ─────────────────────────────────────── */
    /* wall_ve stores TWO entries per wall (line and line-1), so we draw
       each entry as one cell tall — together they cover exactly 2 cells.   */
    for (int i = 0; i < walls * 4; i++) {
        if (wall_ve[i][0] == 0 && wall_ve[i][1] == 0) continue;
        int col  = wall_ve[i][0];
        int line = wall_ve[i][1];
        int px = MARGIN + col * CELL - WALL_THICK / 2;
        int py = MARGIN + (size - line) * CELL;
        SDL_Rect wall = {px, py, WALL_THICK, CELL};
        SDL_SetRenderDrawColor(g_ren, 0x34, 0x98, 0xdb, 255);
        SDL_RenderFillRect(g_ren, &wall);
    }

    /* ── pawns ──────────────────────────────────────────────────────────── */
    int radius = CELL / 2 - 8;

    /* black pawn — dark fill, gold outline */
    int bx = col_to_px(rowb);
    int by = line_to_py(lineb, size);
    SDL_SetRenderDrawColor(g_ren, 0xff, 0xaa, 0x00, 255);
    draw_filled_circle(bx, by, radius + 3);
    SDL_SetRenderDrawColor(g_ren, 0x22, 0x22, 0x22, 255);
    draw_filled_circle(bx, by, radius);

    /* white pawn — light fill, gold outline */
    int wx = col_to_px(roww);
    int wy = line_to_py(linew, size);
    SDL_SetRenderDrawColor(g_ren, 0xff, 0xaa, 0x00, 255);
    draw_filled_circle(wx, wy, radius + 3);
    SDL_SetRenderDrawColor(g_ren, 0xee, 0xee, 0xee, 255);
    draw_filled_circle(wx, wy, radius);

    /* ── info panel ─────────────────────────────────────────────────────── */
    int bpx = MARGIN * 2 + size * CELL;
    SDL_Rect panel = {bpx, 0, PANEL_W, MARGIN * 2 + size * CELL + 30};
    SDL_SetRenderDrawColor(g_ren, 0x16, 0x21, 0x3e, 255);
    SDL_RenderFillRect(g_ren, &panel);

    char buf[64];
    int tx = bpx + 15;

    draw_text_big("QUORIDOR",          tx, 20,  255, 170,   0);
    draw_text("● Black pawn",          tx, 70,  180, 180, 180);
    snprintf(buf, sizeof(buf), "  Walls: %d", walls_b);
    draw_text(buf,                     tx, 90,  180, 180, 180);
    draw_text("○ White pawn",          tx, 125, 180, 180, 180);
    snprintf(buf, sizeof(buf), "  Walls: %d", walls_w);
    draw_text(buf,                     tx, 145, 180, 180, 180);

    draw_text("─────────────────",     tx, 185,  80,  80, 100);
    draw_text("Click cell  → move",    tx, 205, 200, 200, 200);
    draw_text("Click gap   → wall",    tx, 225, 200, 200, 200);
    draw_text("[H] toggle H / V wall", tx, 245, 200, 200, 200);
    draw_text("[G] AI move",           tx, 265, 200, 200, 200);
    draw_text("[Q] quit",              tx, 285, 200, 200, 200);

    /* NOTE: SDL_RenderPresent is NOT called here — caller draws the hover
       preview on top first, then calls draw_preview() which presents.      */
}

/* ── public: draw hover preview on top of board, then present the frame ─── */
/* hover_type: 0=pawn, 1=horizontal wall, 2=vertical wall, -1=nothing        */
void draw_preview(int hover_type, int col, int line, int size, int is_black) {
    if (hover_type == -1 || col == 0 || line == 0) {
        SDL_RenderPresent(g_ren);
        return;
    }

    if (hover_type == 0) {
        /* ghost pawn — semi-transparent at the target cell */
        int cx = col_to_px(col);
        int cy = line_to_py(line, size);
        int radius = CELL / 2 - 8;
        SDL_SetRenderDrawColor(g_ren, 0xff, 0xaa, 0x00, 100);
        draw_filled_circle(cx, cy, radius + 5);
        if (is_black)
            SDL_SetRenderDrawColor(g_ren, 0x22, 0x22, 0x22, 160);
        else
            SDL_SetRenderDrawColor(g_ren, 0xee, 0xee, 0xee, 160);
        draw_filled_circle(cx, cy, radius);
    }
    else if (hover_type == 1) {
        /* ghost horizontal wall — red transparent bar */
        int px = MARGIN + (col - 1) * CELL;
        int py = MARGIN + (size - line) * CELL + CELL - WALL_THICK / 2;
        SDL_Rect wall = {px, py, CELL * 2, WALL_THICK};
        SDL_SetRenderDrawColor(g_ren, 0xe7, 0x4c, 0x3c, 140);
        SDL_RenderFillRect(g_ren, &wall);
    }
    else if (hover_type == 2) {
        /* ghost vertical wall — blue transparent bars (two cell segments) */
        int px = MARGIN + col * CELL - WALL_THICK / 2;
        SDL_Rect wall1 = {px, MARGIN + (size - line) * CELL,     WALL_THICK, CELL};
        SDL_Rect wall2 = {px, MARGIN + (size - line + 1) * CELL, WALL_THICK, CELL};
        SDL_SetRenderDrawColor(g_ren, 0x34, 0x98, 0xdb, 140);
        SDL_RenderFillRect(g_ren, &wall1);
        SDL_RenderFillRect(g_ren, &wall2);
    }

    SDL_RenderPresent(g_ren);
}

/* ── public: pixel → board column (0 if outside) ───────────────────────── */
int px_to_col(int px) {
    int c = (px - MARGIN) / CELL + 1;
    return (c >= 1 && c <= g_size) ? c : 0;
}

/* ── public: pixel → board line (0 if outside) ─────────────────────────── */
int py_to_line(int py) {
    int l = g_size - (py - MARGIN) / CELL;
    return (l >= 1 && l <= g_size) ? l : 0;
}

/* ── public: detect if click landed in a wall gap ───────────────────────── */
/*  Returns 1 = horizontal wall, 2 = vertical wall, 0 = cell click.
    Sets *col and *line to the anchor square.                               */
int px_to_wall_gap(int px, int py, int *col, int *line) {
    if (px < MARGIN || py < MARGIN) return 0;
    int gap = WALL_THICK + 4;
    int rel_x = (px - MARGIN) % CELL;
    int rel_y = (py - MARGIN) % CELL;
    int c = (px - MARGIN) / CELL + 1;
    int l = g_size - (py - MARGIN) / CELL;
    if (c < 1 || c > g_size || l < 1 || l > g_size) return 0;

    /* bottom edge of cell → horizontal wall */
    if (rel_y > CELL - gap && l > 1) {
        *col = c; *line = l;
        return 1;
    }
    /* right edge of cell → vertical wall */
    if (rel_x > CELL - gap && c < g_size) {
        *col = c; *line = l;
        return 2;
    }
    return 0;
}