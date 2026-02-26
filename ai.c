#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"

#define INF 100000

/* ═══════════════════════════════════════════════════════════════════════════
   WALL BLOCKING CHECK
   ═══════════════════════════════════════════════════════════════════════════ */

static int move_blocked(int fc, int fl, int tc, int tl,
                        int hor[][2], int hc,
                        int ver[][2], int vc) {
    int dc = tc - fc, dl = tl - fl;
    if (dl == 1) {
        for (int i = 0; i < hc; i++)
            if (hor[i][1] == tl && (hor[i][0] == fc || hor[i][0] == fc - 1))
                return 1;
    } else if (dl == -1) {
        for (int i = 0; i < hc; i++)
            if (hor[i][1] == fl && (hor[i][0] == fc || hor[i][0] == fc - 1))
                return 1;
    } else if (dc == 1) {
        for (int i = 0; i < vc; i++)
            if (ver[i][0] == fc && ver[i][1] == fl)
                return 1;
    } else if (dc == -1) {
        for (int i = 0; i < vc; i++)
            if (ver[i][0] == tc && ver[i][1] == fl)
                return 1;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
   BFS — returns shortest distance, or INF if unreachable.
   Also optionally fills path[] with the squares on the shortest path
   (path must be size*size long; path_len receives actual length).
   Pass NULL for path/path_len if you just want the distance.
   ═══════════════════════════════════════════════════════════════════════════ */
static int bfs(int sc, int sl, int goal_line,
               int op_c, int op_l, int size,
               int hor[][2], int hc,
               int ver[][2], int vc,
               int path_c[], int path_l[], int *path_len) {

    int par_c[size+2][size+2], par_l[size+2][size+2];
    char vis  [size+2][size+2];
    int  dist [size+2][size+2];
    memset(vis, 0, sizeof(vis));
    for (int i=0;i<=size+1;i++) for(int j=0;j<=size+1;j++){
        par_c[i][j]=-1; par_l[i][j]=-1; dist[i][j]=INF;
    }

    int qc[size*size+8], ql[size*size+8];
    int head=0, tail=0;
    qc[tail]=sc; ql[tail]=sl; tail++;
    vis[sc][sl]=1; dist[sc][sl]=0;

    int dc[]={0,0,1,-1}, dl[]={1,-1,0,0};
    int gc=-1, gl=-1;

    while (head < tail) {
        int cc=qc[head], cl=ql[head]; head++;
        if (cl==goal_line) { gc=cc; gl=cl; break; }

        for (int d=0;d<4;d++) {
            int nc=cc+dc[d], nl=cl+dl[d];
            if (nc<1||nc>size||nl<1||nl>size) continue;
            if (move_blocked(cc,cl,nc,nl,hor,hc,ver,vc)) continue;

            if (nc==op_c && nl==op_l) {
                /* try to jump */
                int jc=nc+dc[d], jl=nl+dl[d];
                if (jc>=1&&jc<=size&&jl>=1&&jl<=size
                    && !vis[jc][jl]
                    && !move_blocked(nc,nl,jc,jl,hor,hc,ver,vc)) {
                    vis[jc][jl]=1;
                    dist[jc][jl]=dist[cc][cl]+1;
                    par_c[jc][jl]=cc; par_l[jc][jl]=cl;
                    qc[tail]=jc; ql[tail]=jl; tail++;
                }
                continue;
            }
            if (!vis[nc][nl]) {
                vis[nc][nl]=1;
                dist[nc][nl]=dist[cc][cl]+1;
                par_c[nc][nl]=cc; par_l[nc][nl]=cl;
                qc[tail]=nc; ql[tail]=nl; tail++;
            }
        }
    }

    if (gc==-1) { if(path_len)*path_len=0; return INF; }

    /* reconstruct path */
    if (path_c && path_l && path_len) {
        int tmp_c[size*size+4], tmp_l[size*size+4];
        int len=0;
        int tc=gc, tl=gl;
        while (tc!=sc || tl!=sl) {
            tmp_c[len]=tc; tmp_l[len]=tl; len++;
            int pc=par_c[tc][tl], pl=par_l[tc][tl];
            tc=pc; tl=pl;
        }
        /* reverse */
        for (int i=0;i<len;i++){
            path_c[i]=tmp_c[len-1-i];
            path_l[i]=tmp_l[len-1-i];
        }
        *path_len=len;
    }

    return dist[gc][gl];
}

/* ═══════════════════════════════════════════════════════════════════════════
   WALL HELPERS
   ═══════════════════════════════════════════════════════════════════════════ */

static int wall_h_overlaps(int col, int line, int hor[][2], int hc) {
    for (int i=0;i<hc;i++)
        if (hor[i][1]==line &&
            (hor[i][0]==col || hor[i][0]==col-1 || hor[i][0]==col+1))
            return 1;
    return 0;
}

static int wall_v_overlaps(int col, int line, int ver[][2], int vc) {
    for (int i=0;i<vc;i++)
        if (ver[i][0]==col && (ver[i][1]==line || ver[i][1]==line-1))
            return 1;
    /* also check 4-consecutive */
    int count=0;
    for (int i=0;i<vc;i++){
        if (ver[i][0]==col){count++; if(count>=4)return 1;}
        else count=0;
    }
    return 0;
}

/* Both players must still have a path after placing the wall */
static int wall_blocks_anyone(int ai_c, int ai_l, int ai_goal,
                               int op_c, int op_l, int op_goal,
                               int size,
                               int hor[][2], int hc,
                               int ver[][2], int vc) {
    int ai_d = bfs(ai_c, ai_l, ai_goal, op_c, op_l, size, hor, hc, ver, vc, NULL, NULL, NULL);
    int op_d = bfs(op_c, op_l, op_goal, ai_c, ai_l, size, hor, hc, ver, vc, NULL, NULL, NULL);
    return (ai_d >= INF || op_d >= INF);
}

/* ═══════════════════════════════════════════════════════════════════════════
   SMART WALL SELECTION
   Find the wall that most increases the opponent's path length,
   chosen from positions that cross the opponent's current shortest path.
   Returns 1 if a good wall was found.
   ═══════════════════════════════════════════════════════════════════════════ */
static int find_best_wall(int ai_c, int ai_l, int ai_goal,
                          int op_c, int op_l, int op_goal,
                          int size,
                          int hor[][2], int hc,
                          int ver[][2], int vc,
                          int *best_type,  /* 1=horizontal, 2=vertical */
                          int *best_col, int *best_line) {

    int op_dist_before = bfs(op_c, op_l, op_goal, ai_c, ai_l,
                              size, hor, hc, ver, vc, NULL, NULL, NULL);

    /* get opponent's current path to find candidate wall positions */
    int path_c[size*size+4], path_l[size*size+4], path_len=0;
    bfs(op_c, op_l, op_goal, ai_c, ai_l, size, hor, hc, ver, vc,
        path_c, path_l, &path_len);

    int best_gain = 1;  /* only place if it adds at least 2 steps */
    *best_type = 0;

    /* try walls near each step of the opponent's path */
    for (int p=0; p<path_len; p++) {
        int pc = path_c[p], pl = path_l[p];

        /* candidate horizontal walls around this path square */
        for (int dc=-1; dc<=1; dc++) {
            int wc = pc + dc;
            for (int wl = pl; wl <= pl+1; wl++) {
                if (wc < 1 || wc+1 > size) continue;
                if (wl < 2 || wl > size)   continue;
                if (wall_h_overlaps(wc, wl, hor, hc)) continue;

                /* temporarily place */
                int tmp_h = hc + 1;
                int new_hor[40][2];
                memcpy(new_hor, hor, sizeof(int)*hc*2);
                new_hor[hc][0]=wc; new_hor[hc][1]=wl;

                if (wall_blocks_anyone(ai_c,ai_l,ai_goal,
                                       op_c,op_l,op_goal,
                                       size,new_hor,tmp_h,ver,vc)) continue;

                int new_dist = bfs(op_c,op_l,op_goal,ai_c,ai_l,
                                   size,new_hor,tmp_h,ver,vc,NULL,NULL,NULL);
                int gain = new_dist - op_dist_before;
                if (gain > best_gain) {
                    best_gain  = gain;
                    *best_type = 1;
                    *best_col  = wc;
                    *best_line = wl;
                }
            }
        }

        /* candidate vertical walls around this path square */
        for (int dl=-1; dl<=1; dl++) {
            int wl = pl + dl;
            for (int wc = pc-1; wc <= pc; wc++) {
                if (wc < 1 || wc+1 > size) continue;
                if (wl < 2 || wl > size)   continue;
                if (wall_v_overlaps(wc, wl, ver, vc)) continue;

                int tmp_v = vc + 2;
                int new_ver[80][2];
                memcpy(new_ver, ver, sizeof(int)*vc*2);
                new_ver[vc][0]=wc;   new_ver[vc][1]=wl;
                new_ver[vc+1][0]=wc; new_ver[vc+1][1]=wl-1;

                if (wall_blocks_anyone(ai_c,ai_l,ai_goal,
                                       op_c,op_l,op_goal,
                                       size,hor,hc,new_ver,tmp_v)) continue;

                int new_dist = bfs(op_c,op_l,op_goal,ai_c,ai_l,
                                   size,hor,hc,new_ver,tmp_v,NULL,NULL,NULL);
                int gain = new_dist - op_dist_before;
                if (gain > best_gain) {
                    best_gain  = gain;
                    *best_type = 2;
                    *best_col  = wc;
                    *best_line = wl;
                }
            }
        }
    }

    return (*best_type != 0);
}

/* ═══════════════════════════════════════════════════════════════════════════
   PUBLIC ENTRY POINT
   ═══════════════════════════════════════════════════════════════════════════ */
void ai_generate(int *row, int *line,
                 int op_row, int op_line,
                 int hor[][2], int hc,
                 int ver[][2], int vc,
                 int goal_line, int size,
                 int ai_walls_left, int op_walls_left,
                 int hor_out[][2], int *hc_out,
                 int ver_out[][2], int *vc_out) {

    (void)op_walls_left;

    int ai_goal = goal_line;
    int op_goal = (goal_line == 1) ? size : 1;

    int ai_dist = bfs(*row, *line, ai_goal, op_row, op_line,
                      size, hor, hc, ver, vc, NULL, NULL, NULL);
    int op_dist = bfs(op_row, op_line, op_goal, *row, *line,
                      size, hor, hc, ver, vc, NULL, NULL, NULL);

    /* ── decide: place wall or move? ───────────────────────────────────────
       Place a wall when:
       - We have walls left
       - Opponent is close to winning (within 4 steps) OR opponent is ahead of us
       - A wall exists that meaningfully slows them down (adds ≥2 steps)       */
    int should_wall = 0;
    int w_type=0, w_col=0, w_line=0;

    if (ai_walls_left > 0 && (op_dist <= 4 || op_dist < ai_dist)) {
        should_wall = find_best_wall(*row, *line, ai_goal,
                                     op_row, op_line, op_goal,
                                     size, hor, hc, ver, vc,
                                     &w_type, &w_col, &w_line);
    }

    if (should_wall) {
        /* place the wall */
        if (w_type == 1) {
            hor_out[*hc_out][0] = w_col;
            hor_out[*hc_out][1] = w_line;
            (*hc_out)++;
        } else {
            ver_out[*vc_out][0] = w_col;   ver_out[*vc_out][1] = w_line;   (*vc_out)++;
            ver_out[*vc_out][0] = w_col;   ver_out[*vc_out][1] = w_line-1; (*vc_out)++;
        }
        return;  /* wall placed — pawn stays */
    }

    /* ── move pawn along BFS shortest path ─────────────────────────────────
       Anti-oscillation: get the full path and pick the first step.
       If only one move is available (trapped), take it regardless.            */
    int path_c[size*size+4], path_l[size*size+4], path_len=0;
    bfs(*row, *line, ai_goal, op_row, op_line, size, hor, hc, ver, vc,
        path_c, path_l, &path_len);

    if (path_len > 0) {
        *row  = path_c[0];
        *line = path_l[0];
    }
    /* if path_len == 0, no path exists — stay put (shouldn't happen) */
}