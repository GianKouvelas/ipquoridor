#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <math.h>
#include "header.h"

/* ── layout ─────────────────────────────────────────────────────────────── */
#define CELL        72
#define WALL_THICK  12
#define WALL_R      6        /* wall bar corner "radius" (drawn as inset)    */
#define MARGIN      52
#define PANEL_W     230

/* ── palette — luxury dark ──────────────────────────────────────────────── */
/* background */
#define C_BG_R   0x12, 0x14, 0x1a          /* near-black slate               */
/* board surround */
#define C_SURR_R 0x1c, 0x1f, 0x28          /* dark slate frame               */
/* cells — two alternating tones */
#define C_CELL_A 0x2a, 0x2d, 0x38          /* dark cell                      */
#define C_CELL_B 0x30, 0x34, 0x42          /* slightly lighter cell          */
/* cell inner highlight (top-left edge) */
#define C_CELL_HL 0x3a, 0x3e, 0x52
/* grid lines */
#define C_GRID   0x1a, 0x1c, 0x24
/* labels */
#define C_LABEL  0x55, 0x5a, 0x70
/* horizontal wall — warm amber */
#define C_WALL_H   0xf0, 0x9a, 0x1e        /* amber                          */
#define C_WALL_H_L 0xff, 0xc8, 0x60        /* amber highlight                */
/* vertical wall — cool teal */
#define C_WALL_V   0x1a, 0xb8, 0x9a        /* teal                           */
#define C_WALL_V_L 0x5c, 0xe8, 0xcc        /* teal highlight                 */
/* pawn colours */
#define C_PAWN_B   0x18, 0x18, 0x20        /* dark pawn body                 */
#define C_PAWN_W   0xec, 0xec, 0xf4        /* light pawn body                */
#define C_PAWN_HL  0xff, 0xff, 0xff        /* specular highlight             */
#define C_PAWN_RIM 0xd4, 0xa8, 0x17        /* gold rim for both pawns        */
/* panel */
#define C_PANEL    0x16, 0x19, 0x22
#define C_DIVIDER  0x2a, 0x2e, 0x3e
/* text */
#define C_TEXT_HI  0xf0, 0xb9, 0x2a        /* amber title                    */
#define C_TEXT_MD  0xb0, 0xb4, 0xc8        /* medium labels                  */
#define C_TEXT_DIM 0x55, 0x5a, 0x72        /* dim hints                      */
/* wall pip colours in sidebar */
#define C_PIP_H    0xf0, 0x9a, 0x1e
#define C_PIP_V    0x1a, 0xb8, 0x9a
#define C_PIP_USED 0x2a, 0x2d, 0x38

/* ── state ──────────────────────────────────────────────────────────────── */
static SDL_Window   *g_win      = NULL;
static SDL_Renderer *g_ren      = NULL;
static TTF_Font     *g_font     = NULL;
static TTF_Font     *g_font_big = NULL;
static TTF_Font     *g_font_sm  = NULL;
static int           g_size     = 0;

/* ── primitives ─────────────────────────────────────────────────────────── */

static int col_to_px(int col) {
    return MARGIN + (col - 1) * CELL + CELL / 2;
}
static int line_to_py(int line, int size) {
    return MARGIN + (size - line) * CELL + CELL / 2;
}

static void set_col(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(g_ren, r, g, b, a);
}

static void draw_text_col(TTF_Font *font, const char *text,
                           int x, int y, Uint8 r, Uint8 g, Uint8 b) {
    if (!font) return;
    SDL_Color c = {r, g, b, 255};
    SDL_Surface *s = TTF_RenderText_Blended(font, text, c);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(g_ren, s);
    SDL_Rect dst = {x, y, s->w, s->h};
    SDL_RenderCopy(g_ren, t, NULL, &dst);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(s);
}
static void draw_text(const char *txt, int x, int y, Uint8 r, Uint8 g, Uint8 b) {
    draw_text_col(g_font, txt, x, y, r, g, b);
}
static void draw_text_big(const char *txt, int x, int y, Uint8 r, Uint8 g, Uint8 b) {
    draw_text_col(g_font_big, txt, x, y, r, g, b);
}
static void draw_text_sm(const char *txt, int x, int y, Uint8 r, Uint8 g, Uint8 b) {
    draw_text_col(g_font_sm, txt, x, y, r, g, b);
}

/* filled circle */
static void fill_circle(int cx, int cy, int rad) {
    for (int dy = -rad; dy <= rad; dy++) {
        int dx = (int)sqrt((double)(rad*rad - dy*dy));
        SDL_RenderDrawLine(g_ren, cx-dx, cy+dy, cx+dx, cy+dy);
    }
}

/* ring outline (annulus) — for rims */
static void draw_ring(int cx, int cy, int outer, int inner) {
    for (int dy = -outer; dy <= outer; dy++) {
        int dxo = (int)sqrt((double)(outer*outer - dy*dy));
        int dxi = (int)(abs(dy) <= inner
                        ? sqrt((double)(inner*inner - dy*dy))
                        : 0);
        if (dxo > dxi) {
            SDL_RenderDrawLine(g_ren, cx-dxo, cy+dy, cx-dxi, cy+dy);
            SDL_RenderDrawLine(g_ren, cx+dxi, cy+dy, cx+dxo, cy+dy);
        }
    }
}

/* rounded-end wall bar */
static void fill_wall_h(int x, int y, int w, int h) {
    /* main bar */
    SDL_Rect r = {x, y, w, h};
    SDL_RenderFillRect(g_ren, &r);
}
static void fill_wall_v(int x, int y, int w, int h) {
    SDL_Rect r = {x, y, w, h};
    SDL_RenderFillRect(g_ren, &r);
}

/* ── init / quit ─────────────────────────────────────────────────────────── */

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

    /* font paths for Ubuntu/Debian — fallback gracefully if missing */
    g_font_big = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20);
    g_font     = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",      14);
    g_font_sm  = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",      11);
}

void sdl_quit(void) {
    if (g_font_sm)  TTF_CloseFont(g_font_sm);
    if (g_font)     TTF_CloseFont(g_font);
    if (g_font_big) TTF_CloseFont(g_font_big);
    if (g_ren)      SDL_DestroyRenderer(g_ren);
    if (g_win)      SDL_DestroyWindow(g_win);
    TTF_Quit();
    SDL_Quit();
}

/* ── pawn drawing — 3-layer: rim → body → specular highlight ────────────── */
static void draw_pawn(int cx, int cy, int is_black) {
    int rad = CELL / 2 - 9;

    /* drop shadow */
    set_col(0x00, 0x00, 0x00, 80);
    fill_circle(cx + 2, cy + 3, rad + 3);

    /* gold rim */
    set_col(C_PAWN_RIM, 255);
    fill_circle(cx, cy, rad + 3);

    /* body */
    if (is_black) set_col(C_PAWN_B, 255);
    else          set_col(C_PAWN_W, 255);
    fill_circle(cx, cy, rad);

    /* inner shading ring for depth */
    if (is_black) set_col(0x08, 0x08, 0x10, 120);
    else          set_col(0xc0, 0xc0, 0xd0, 80);
    draw_ring(cx, cy, rad, rad - 4);

    /* specular highlight — top-left quarter */
    set_col(C_PAWN_HL, is_black ? 90 : 160);
    fill_circle(cx - rad/3, cy - rad/3, rad/3);

    /* softer outer glow on specular */
    set_col(C_PAWN_HL, is_black ? 40 : 80);
    fill_circle(cx - rad/3, cy - rad/3, rad/2);
}

/* ── wall drawing — bar + inner highlight stripe ────────────────────────── */
static void draw_wall_h_styled(int x, int y, int w, int h) {
    /* main bar */
    set_col(C_WALL_H, 255);
    fill_wall_h(x, y, w, h);
    /* top highlight stripe */
    set_col(C_WALL_H_L, 180);
    fill_wall_h(x + 2, y + 1, w - 4, 3);
    /* subtle dark bottom edge */
    set_col(0x00, 0x00, 0x00, 60);
    fill_wall_h(x, y + h - 2, w, 2);
}

static void draw_wall_v_styled(int x, int y, int w, int h) {
    set_col(C_WALL_V, 255);
    fill_wall_v(x, y, w, h);
    /* left highlight stripe */
    set_col(C_WALL_V_L, 180);
    fill_wall_v(x + 1, y + 2, 3, h - 4);
    /* dark right edge */
    set_col(0x00, 0x00, 0x00, 60);
    fill_wall_v(x + w - 2, y, 2, h);
}

/* ── wall pip row for sidebar (shows remaining walls as coloured dots) ───── */
static void draw_wall_pips(int tx, int ty, int total, int remaining, int is_h) {
    int pip = 10;
    int gap = 4;
    for (int i = 0; i < total; i++) {
        int px = tx + i * (pip + gap);
        SDL_Rect r = {px, ty, pip, pip};
        if (i < remaining) {
            if (is_h) set_col(C_PIP_H, 255);
            else      set_col(C_PIP_V, 255);
        } else {
            set_col(C_PIP_USED, 255);
        }
        SDL_RenderFillRect(g_ren, &r);
    }
}

/* ── main draw ───────────────────────────────────────────────────────────── */
void display_board(int size, int walls_w, int walls_b,
                   int linew, int roww, int lineb, int rowb,
                   int walls, int wall_ho[][2], int wall_ve[][2]) {

    int board_px = MARGIN + size * CELL;

    /* ── background ─────────────────────────────────────────────────────── */
    set_col(C_BG_R, 255);
    SDL_RenderClear(g_ren);

    /* ── sidebar panel (draw first so board content renders on top) ──────── */
    SDL_Rect panel = {board_px, 0, PANEL_W, MARGIN * 2 + size * CELL + 30};
    set_col(C_PANEL, 255);
    SDL_RenderFillRect(g_ren, &panel);

    /* panel left border */
    set_col(C_DIVIDER, 255);
    SDL_RenderDrawLine(g_ren, board_px, 0,
                       board_px, MARGIN * 2 + size * CELL + 30);

    /* ── board surround / frame ──────────────────────────────────────────── */
    SDL_Rect frame = {MARGIN - 6, MARGIN - 6,
                      size * CELL + 12, size * CELL + 12};
    set_col(C_SURR_R, 255);
    SDL_RenderFillRect(g_ren, &frame);

    /* ── cells ───────────────────────────────────────────────────────────── */
    for (int col = 1; col <= size; col++) {
        for (int line = 1; line <= size; line++) {
            int px = MARGIN + (col - 1) * CELL;
            int py = MARGIN + (size - line) * CELL;

            /* checkerboard shading */
            if ((col + line) % 2 == 0)  set_col(C_CELL_A, 255);
            else                         set_col(C_CELL_B, 255);
            SDL_Rect cell = {px, py, CELL, CELL};
            SDL_RenderFillRect(g_ren, &cell);

            /* subtle top-left inner highlight edge */
            set_col(C_CELL_HL, 60);
            SDL_RenderDrawLine(g_ren, px, py, px + CELL - 1, py);   /* top  */
            SDL_RenderDrawLine(g_ren, px, py, px, py + CELL - 1);   /* left */

            /* grid line (right + bottom) */
            set_col(C_GRID, 255);
            SDL_RenderDrawLine(g_ren, px + CELL, py, px + CELL, py + CELL);
            SDL_RenderDrawLine(g_ren, px, py + CELL, px + CELL, py + CELL);
        }
    }

    /* ── labels ──────────────────────────────────────────────────────────── */
    char lbl[4];
    for (int col = 1; col <= size; col++) {
        lbl[0] = 'A' + col - 1; lbl[1] = '\0';
        int px = MARGIN + (col - 1) * CELL + CELL / 2 - 4;
        draw_text_sm(lbl, px, MARGIN + size * CELL + 8,  0x55, 0x5a, 0x70);
        draw_text_sm(lbl, px, MARGIN - 20,               0x55, 0x5a, 0x70);
    }
    for (int line = 1; line <= size; line++) {
        snprintf(lbl, sizeof(lbl), "%d", line);
        int py = MARGIN + (size - line) * CELL + CELL / 2 - 7;
        draw_text_sm(lbl, MARGIN - 22, py, 0x55, 0x5a, 0x70);
        draw_text_sm(lbl, MARGIN + size * CELL + 10, py, 0x55, 0x5a, 0x70);
    }

    /* ── horizontal walls ────────────────────────────────────────────────── */
    for (int i = 0; i < walls * 2; i++) {
        if (wall_ho[i][0] == 0 && wall_ho[i][1] == 0) continue;
        int col  = wall_ho[i][0];
        int line = wall_ho[i][1];
        int px = MARGIN + (col - 1) * CELL;
        int py = MARGIN + (size - line) * CELL + CELL - WALL_THICK / 2;
        draw_wall_h_styled(px, py, CELL * 2, WALL_THICK);
    }

    /* ── vertical walls ──────────────────────────────────────────────────── */
    for (int i = 0; i < walls * 4; i++) {
        if (wall_ve[i][0] == 0 && wall_ve[i][1] == 0) continue;
        int col  = wall_ve[i][0];
        int line = wall_ve[i][1];
        int px = MARGIN + col * CELL - WALL_THICK / 2;
        int py = MARGIN + (size - line) * CELL;
        draw_wall_v_styled(px, py, WALL_THICK, CELL);
    }

    /* ── pawns ───────────────────────────────────────────────────────────── */
    draw_pawn(col_to_px(rowb), line_to_py(lineb, size), 1);
    draw_pawn(col_to_px(roww), line_to_py(linew, size), 0);

    /* ── sidebar text content ────────────────────────────────────────────── */
    int tx = board_px + 75;
    char buf[64];

    draw_text_big("QUORIDOR", tx, 22, 0xf0, 0xb9, 0x2a);

    set_col(C_DIVIDER, 255);
    SDL_Rect div1 = {tx, 56, PANEL_W - 36, 1};
    SDL_RenderFillRect(g_ren, &div1);

    draw_text("BLACK", tx, 68, 0xb0, 0xb4, 0xc8);
    snprintf(buf, sizeof(buf), "%d wall%s left", walls_b, walls_b == 1 ? "" : "s");
    draw_text_sm(buf, tx, 86, 0x55, 0x5a, 0x72);
    draw_wall_pips(tx, 104, walls, walls_b, 1);

    set_col(C_DIVIDER, 255);
    SDL_Rect div2 = {tx, 124, PANEL_W - 36, 1};
    SDL_RenderFillRect(g_ren, &div2);

    draw_text("WHITE", tx, 136, 0xb0, 0xb4, 0xc8);
    snprintf(buf, sizeof(buf), "%d wall%s left", walls_w, walls_w == 1 ? "" : "s");
    draw_text_sm(buf, tx, 154, 0x55, 0x5a, 0x72);
    draw_wall_pips(tx, 172, walls, walls_w, 0);

    set_col(C_DIVIDER, 255);
    SDL_Rect div3 = {tx, 196, PANEL_W - 36, 1};
    SDL_RenderFillRect(g_ren, &div3);

    draw_text_sm("click cell   move pawn",  tx, 208, 0x55, 0x5a, 0x72);
    draw_text_sm("click gap    place wall", tx, 224, 0x55, 0x5a, 0x72);
    draw_text_sm("[G]  AI move",            tx, 240, 0x55, 0x5a, 0x72);
    draw_text_sm("[Q]  quit",               tx, 256, 0x55, 0x5a, 0x72);

    /* NOTE: SDL_RenderPresent not called here — draw_preview() does it */
}

/* ── hover preview ───────────────────────────────────────────────────────── */
void draw_preview(int hover_type, int col, int line, int size, int is_black) {
    if (hover_type == -1 || col == 0 || line == 0) {
        SDL_RenderPresent(g_ren);
        return;
    }

    if (hover_type == 0) {
        /* ghost pawn — pulsing ring + translucent body */
        int cx = col_to_px(col);
        int cy = line_to_py(line, size);
        int rad = CELL / 2 - 9;

        set_col(C_PAWN_RIM, 70);
        fill_circle(cx, cy, rad + 5);
        if (is_black) set_col(C_PAWN_B, 130);
        else          set_col(C_PAWN_W, 130);
        fill_circle(cx, cy, rad);
    }
    else if (hover_type == 1) {
        /* ghost horizontal wall */
        int px = MARGIN + (col - 1) * CELL;
        int py = MARGIN + (size - line) * CELL + CELL - WALL_THICK / 2;
        set_col(C_WALL_H, 110);
        SDL_Rect wall = {px, py, CELL * 2, WALL_THICK};
        SDL_RenderFillRect(g_ren, &wall);
        set_col(C_WALL_H_L, 80);
        SDL_Rect hl = {px + 2, py + 1, CELL * 2 - 4, 3};
        SDL_RenderFillRect(g_ren, &hl);
    }
    else if (hover_type == 2) {
        /* ghost vertical wall */
        int px = MARGIN + col * CELL - WALL_THICK / 2;
        set_col(C_WALL_V, 110);
        SDL_Rect w1 = {px, MARGIN + (size - line) * CELL,     WALL_THICK, CELL};
        SDL_Rect w2 = {px, MARGIN + (size - line + 1) * CELL, WALL_THICK, CELL};
        SDL_RenderFillRect(g_ren, &w1);
        SDL_RenderFillRect(g_ren, &w2);
        set_col(C_WALL_V_L, 80);
        SDL_Rect h1 = {px + 1, MARGIN + (size - line) * CELL + 2,     3, CELL - 4};
        SDL_Rect h2 = {px + 1, MARGIN + (size - line + 1) * CELL + 2, 3, CELL - 4};
        SDL_RenderFillRect(g_ren, &h1);
        SDL_RenderFillRect(g_ren, &h2);
    }

    SDL_RenderPresent(g_ren);
}

/* ── coordinate helpers ──────────────────────────────────────────────────── */
int px_to_col(int px) {
    int c = (px - MARGIN) / CELL + 1;
    return (c >= 1 && c <= g_size) ? c : 0;
}
int py_to_line(int py) {
    int l = g_size - (py - MARGIN) / CELL;
    return (l >= 1 && l <= g_size) ? l : 0;
}
int px_to_wall_gap(int px, int py, int *col, int *line) {
    if (px < MARGIN || py < MARGIN) return 0;
    int gap   = WALL_THICK + 4;
    int rel_x = (px - MARGIN) % CELL;
    int rel_y = (py - MARGIN) % CELL;
    int c     = (px - MARGIN) / CELL + 1;
    int l     = g_size - (py - MARGIN) / CELL;
    if (c < 1 || c > g_size || l < 1 || l > g_size) return 0;
    if (rel_y > CELL - gap && l > 1)    { *col = c; *line = l; return 1; }
    if (rel_x > CELL - gap && c < g_size) { *col = c; *line = l; return 2; }
    return 0;
}