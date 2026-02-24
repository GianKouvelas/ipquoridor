#include "header.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ── BFS to check if (start_col, start_line) can reach goal_line ────────── */
/* Returns 1 if reachable, 0 if blocked.                                      */
static int can_reach(int start_col, int start_line, int goal_line,
                     int size, int hor[][2], int h_count,
                     int ver[][2], int v_count) {

    // printf("  BFS: from col=%d line=%d -> goal_line=%d, hc=%d vc=%d\n",
    //       start_col, start_line, goal_line, h_count, v_count);

    /* visited[col-1][line-1] */
    char visited[size][size];
    memset(visited, 0, sizeof(visited));

    /* simple BFS queue — max size is board squares */
    int qcol[size * size], qline[size * size];
    int head = 0, tail = 0;

    qcol[tail]  = start_col;
    qline[tail] = start_line;
    tail++;
    visited[start_col - 1][start_line - 1] = 1;

    /* 4 directions: right, left, up, down */
    int dc[] = { 1, -1,  0,  0};
    int dl[] = { 0,  0,  1, -1};

    while (head < tail) {
        int cc = qcol[head];
        int cl = qline[head];
        head++;

        if (cl == goal_line) return 1;

        for (int d = 0; d < 4; d++) {
            int nc = cc + dc[d];
            int nl = cl + dl[d];

            if (nc < 1 || nc > size || nl < 1 || nl > size) continue;
            if (visited[nc - 1][nl - 1]) continue;

            /* check horizontal walls blocking vertical movement */
            int blocked = 0;
            if (dl[d] == 1) {
                /* moving up: wall blocks if it sits between cl and nl=cl+1 */
                for (int i = 0; i < h_count && !blocked; i++)
                    if (hor[i][1] == nl &&
                        (hor[i][0] == cc || hor[i][0] == cc - 1))
                        blocked = 1;
            } else if (dl[d] == -1) {
                /* moving down: wall between nl=cl-1 and cl */
                for (int i = 0; i < h_count && !blocked; i++)
                    if (hor[i][1] == cl &&
                        (hor[i][0] == cc || hor[i][0] == cc - 1))
                        blocked = 1;
            } else if (dc[d] == 1) {
                /* moving right: vertical wall between cc and nc=cc+1 */
                for (int i = 0; i < v_count && !blocked; i++)
                    if (ver[i][0] == cc && ver[i][1] == cl)
                        blocked = 1;
            } else if (dc[d] == -1) {
                /* moving left: vertical wall between nc=cc-1 and cc */
                for (int i = 0; i < v_count && !blocked; i++)
                    if (ver[i][0] == nc && ver[i][1] == cl)
                        blocked = 1;
            }

            if (!blocked) {
                visited[nc - 1][nl - 1] = 1;
                qcol[tail]  = nc;
                qline[tail] = nl;
                tail++;
            }
        }
    }
    return 0;
}

/* ── main wall placement function ───────────────────────────────────────── */
int playwall(char array[], int wall_ho[][2], int *h_count,
             int wall_ve[][2], int *v_count, int dimen,
             int size,
             int b_col, int b_line, int w_col, int w_line) {

    int temp1, temp2, count = 0;
    if (array[0] > 90)
        temp1 = array[0] - 96;
    else
        temp1 = array[0] - 64;
    temp2 = array[1] - 48;

    if (dimen == 1) {
        /* ── horizontal wall ── */
        for (int i = 0; i < *h_count; i++) {
            if (wall_ho[i][1] == temp2 &&
                (wall_ho[i][0] == temp1 ||
                 wall_ho[i][0] == temp1 - 1 ||
                 wall_ho[i][0] == temp1 + 1))
                return 1;   /* overlaps existing wall */
        }

        /* place temporarily */
        wall_ho[*h_count][0] = temp1;
        wall_ho[*h_count][1] = temp2;
        (*h_count)++;

        /* BFS check for both players */
        if (!can_reach(b_col, b_line, 1,    size, wall_ho, *h_count, wall_ve, *v_count) ||
            !can_reach(w_col, w_line, size,  size, wall_ho, *h_count, wall_ve, *v_count)) {
            /* undo */
            (*h_count)--;
            wall_ho[*h_count][0] = 0;
            wall_ho[*h_count][1] = 0;
            return 1;
        }

    } else {
        /* ── vertical wall ── */
        for (int i = 0; i < (*v_count) * 2; i++) {
            if (wall_ve[i][0] == temp1) {
                count++;
                if (count == 4) return 1;
            } else {
                count = 0;
            }
        }

        /* place temporarily (two entries) */
        wall_ve[*v_count][0] = temp1;
        wall_ve[*v_count][1] = temp2;
        (*v_count)++;
        wall_ve[*v_count][0] = temp1;
        wall_ve[*v_count][1] = temp2 - 1;
        (*v_count)++;

        /* BFS check */
        if (!can_reach(b_col, b_line, 1,    size, wall_ho, *h_count, wall_ve, *v_count) ||
            !can_reach(w_col, w_line, size,  size, wall_ho, *h_count, wall_ve, *v_count)) {
            /* undo both entries */
            (*v_count) -= 2;
            wall_ve[*v_count][0] = 0;
            wall_ve[*v_count][1] = 0;
            wall_ve[*v_count + 1][0] = 0;
            wall_ve[*v_count + 1][1] = 0;
            return 1;
        }
    }

    return 0;
}