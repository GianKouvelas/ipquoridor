#include "header.h"
#include <stdlib.h>

void insert_at_end(listnode *alist,int row,int line) {
    while (*alist != NULL)
        alist = &((*alist)->next);
    *alist = malloc(sizeof(struct list)); 
        (*alist)->cord1 = row;
        (*alist)->cord2 = line;
        (*alist)->next = NULL; 
}