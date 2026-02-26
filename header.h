struct board {
    int size;
    int walls_w;
    int walls_b;
};

struct player {
    int row;
    int line;
};

typedef struct list *listnode;

struct list {
    int cord1;
    int cord2;
    listnode next;
};

/* ── core game functions ─────────────────────────────────────────────────── */
void insert_at_end(listnode *alist, int row, int line);

void display_board(int size, int walls_w, int walls_b,
                   int linew, int roww, int lineb, int rowb,
                   int walls, int wall_ho[][2], int wall_ve[][2]);

int playmove(char array[], int *row, int *line, int op_row, int op_line,
             int hor[][2], int ver[][2], int count_walls);

int playwall(char array[], int wall_ho[][2], int *h_count,
             int wall_ve[][2], int *v_count, int dimen,
             int size,
             int b_col, int b_line, int w_col, int w_line);

void sorting(int array[][2], int walls);

void ai_generate(int *row, int *line,
                 int op_row, int op_line,
                 int hor[][2], int hc,
                 int ver[][2], int vc,
                 int goal_line, int size,
                 int ai_walls_left, int op_walls_left,
                 int hor_out[][2], int *hc_out,
                 int ver_out[][2], int *vc_out);

/* ── SDL display helpers (defined in Display_board.c) ───────────────────── */
void sdl_init(int board_size);
void sdl_quit(void);
int  px_to_col(int px);
int  py_to_line(int py);
int  px_to_wall_gap(int px, int py, int *col, int *line);
void draw_preview(int hover_type, int col, int line, int size, int is_black);