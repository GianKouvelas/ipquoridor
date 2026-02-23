#include <stdio.h>
#include <stdlib.h>
#include "header.h"

int playmove(char array[], int *row, int *line, int op_row, int op_line,
             int hor[][2], int ver[][2], int count_walls) {

    int i, temp, temp2;

    /* ── decode target square ───────────────────────────────────────────── */
    if (array[0] > 90)
        temp = array[0] - 96;
    else
        temp = array[0] - 64;
    temp2 = array[1] - 48;

    /* ── direction of intended move ─────────────────────────────────────── */
    int dcol  = temp  - *row;
    int dline = temp2 - *line;

    /* ── basic validity ─────────────────────────────────────────────────── */
    if (temp == *row && temp2 == *line) {
        printf("\nSorry wrong move, try again\n");
        return 1;
    }
    if (abs(dcol) > 2 || abs(dline) > 2) {
        printf("\nSorry wrong move, try again\n");
        return 1;
    }
    if (dcol != 0 && dline != 0) {
        printf("\nSorry wrong move, try again\n");
        return 1;
    }

    /* ── jump detection ─────────────────────────────────────────────────── */
    int jumping = 0;
    /* the adjacent square in the direction we're moving */
    int adj_col  = *row  + (dcol  > 0 ? 1 : dcol  < 0 ? -1 : 0);
    int adj_line = *line + (dline > 0 ? 1 : dline < 0 ? -1 : 0);

    if (adj_col == op_row && adj_line == op_line) {
        /* opponent is right next to us in our direction */
        if (abs(dcol) == 1 || abs(dline) == 1) {
            /* clicked on opponent's square → auto jump to the square beyond */
            temp  = op_row  + (dcol  > 0 ? 1 : dcol  < 0 ? -1 : 0);
            temp2 = op_line + (dline > 0 ? 1 : dline < 0 ? -1 : 0);
            dcol  = temp  - *row;
            dline = temp2 - *line;
        }
        jumping = 1;
    } else if ((abs(dcol) == 2 || abs(dline) == 2)) {
        /* clicked 2 squares away — only valid as a jump over the opponent */
        /* check that opponent is actually in the middle square */
        int mid_col  = *row  + (dcol  > 0 ? 1 : -1);
        int mid_line = *line + (dline > 0 ? 1 : -1);
        if (mid_col == op_row && mid_line == op_line) {
            jumping = 1;
        } else {
            printf("\nSorry wrong move, try again\n");
            return 1;
        }
    }

    if (!jumping && (abs(dcol) == 2 || abs(dline) == 2)) {
        printf("\nSorry wrong move, try again\n");
        return 1;
    }

    /* ── wall checks (one step at a time, including both steps of a jump) ── */
    int steps     = jumping ? 2 : 1;
    int step_col  = (dcol  > 0 ? 1 : dcol  < 0 ? -1 : 0);
    int step_line = (dline > 0 ? 1 : dline < 0 ? -1 : 0);
    int cc = *row, cl = *line;

    for (int s = 0; s < steps; s++) {
        int fc = cc, fl = cl;
        int tc = cc + step_col, tl = cl + step_line;

        for (i = 0; i < count_walls; i++) {
            if (((hor[i][1] - fl == 1 && hor[i][1] - tl == 0) ||
                 (hor[i][1] - fl == 0 && hor[i][1] - tl == 1)) &&
                (tc == hor[i][0] || tc == hor[i][0] + 1)) {
                printf("\nSorry wrong move, wall in the way\n");
                return 1;
            }
        }
        for (i = 0; i < count_walls * 2; i++) {
            if (((ver[i][0] - fc == 0  && ver[i][0] - tc == -1) ||
                 (ver[i][0] - fc == -1 && ver[i][0] - tc == 0)) &&
                ver[i][1] == tl) {
                printf("\nSorry wrong move, wall in the way\n");
                return 1;
            }
        }
        cc = tc;
        cl = tl;
    }

    /* ── apply ──────────────────────────────────────────────────────────── */
    *row  = temp;
    *line = temp2;
    return 0;
}