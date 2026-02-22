#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"

int turn=0;


int main(void) {
    char buffer[10],string[20];
    struct board table; //table will be the board and pointers will keep the position for each pawn
    struct player *white,*black;
    struct list *list_mem=NULL; //we are going to use this list to memorise the previous moves
    int walls,i=0,tr,dimen,h=0,v=0;//we are using turns to define which player is moving or placing a wall
    white=malloc(sizeof *white); //allocate dynamically memory for pointers
    black=malloc(sizeof *black);
    printf("\nboardsize : ");
    scanf("%d",&table.size);//giving the size of the board
    printf("=\n\nclear_board\n=\n\nwalls : ");
    scanf("%d",&walls);//giving the number of walls for each player
    table.walls_b=walls;//giiving the max walls for each player
    table.walls_w=walls;
    int wall_ho[walls*2][2],wall_ve[walls*4][2]; // arrays for the locations of the horizontal and the vertical walls
    getchar(); //read from stdin \n
    while(1) {
        printf("=\n\nshowboard\n=\n\n");
        if(white->line==table.size || black->line==1) //condition for game to end successfuly
        {
            if(white->line==table.size)
                printf("\nThe winner is white!!\n");
            else if(black->line==1)
                printf("\nThe winner is black!!\n");
            printf("\nQuit\n = \n");    
            break; //break from the loop if there is a winner  
        }
        if(turn==0){ //when turn is 0 we are at the start of the game and the players go to their starting position
             white->line=1;
             white->row=table.size/2+1;
             black->line=table.size;
             black->row=table.size/2+1;
             insert_at_end(&list_mem,black->row,black->line);
             insert_at_end(&list_mem,white->row,white->line);
        }
        else if (turn%2==1) { //when turn is odd blacks' player decides if he wants to move or if he wants to place a wall
            while(1) {
                printf("black's turn :");
                fgets(string,20,stdin);
                if(strrchr(string,'p')!=NULL || strrchr(string,'P')!=NULL){
                fgets(buffer,10,stdin);
                if(strrchr(buffer,'h')!=NULL || strrchr(buffer,'H')!=NULL) {
                    if(table.walls_b==0){
                        printf("there is no more walls to play\n");
                        continue;
                    }
                    dimen=1; //when dimen is 1 it means its a horizontal wall
                    tr=playwall(buffer,wall_ho,&h,dimen);
                    if(tr==0) { // if playmove returns 0 it means everything went ok
                        table.walls_b--;
                            sorting(wall_ho,h);
                        break;
                    }
                }
                else if(strrchr(buffer,'v')!=NULL || strrchr(buffer,'V')!=NULL) {
                    if(table.walls_b==0){
                        printf("there is no more walls to play\n");
                        continue;
                    }
                    dimen=0; //when dimen is 0 it means its a vertical wall
                    tr=playwall(buffer,wall_ve,&v,dimen);
                    if(tr==0) { // if playmove returns 0 it means everything went ok
                        table.walls_b--;
                            sorting(wall_ve,v);
                        break;
                    }
                }
                else {
                    tr=playmove(buffer,&(black->row),&(black->line),list_mem,wall_ho,wall_ve,2*walls-table.walls_b-table.walls_w);
                    if(tr==0) { // if playmove returns 0 it means everything went ok
                        insert_at_end(&list_mem,black->row,black->line);
                        break;
                    }
                }
            }
            else if(strrchr(string,'g')!=NULL || strrchr(string,'G')!=NULL){
                ai_generate(&(black->row),&(black->line),&(white->row),&(white->line),list_mem,wall_ho,2*walls-table.walls_b,turn,table.size);
                insert_at_end(&list_mem,black->row,black->line);
                break;
            }
                fflush(stdout);
            }
        }
        else { //its white's player turn
            while(1) {
                printf("white's turn :");
                fgets(string,20,stdin);
                if(strrchr(string,'p')!=NULL || strrchr(string,'P')!=NULL){
                fgets(buffer,10,stdin);
                if(strrchr(buffer,'h')!=NULL || strrchr(buffer,'H')!=NULL) {
                    if(table.walls_w==0){
                        printf("there is no more walls to play\n");
                        continue;
                    }
                    dimen=1;
                    tr=playwall(buffer,wall_ho,&h,dimen);
                    if(tr==0) { // if playmove returns 0 it means everything went ok
                        table.walls_w--;
                            sorting(wall_ho,h);
                        break;
                    }
                }
                else if(strrchr(buffer,'v')!=NULL || strrchr(buffer,'V')!=NULL) {
                    if(table.walls_w==0){
                        printf("there is no more walls to play\n");
                        continue;
                    }
                    dimen=0; //when dimen is 0 it means its a vertical wall
                    tr=playwall(buffer,wall_ve,&v,dimen);
                    if(tr==0) { // if playmove returns 0 it means everything went ok
                        table.walls_w--;
                            sorting(wall_ve,v);
                        break;
                    }
                }
                else {
                    tr=playmove(buffer,&(white->row),&(white->line),list_mem,wall_ho,wall_ve,2*walls-table.walls_b-table.walls_w);
                    if(tr==0) { // if playmove returns 0 it means everything went ok
                        insert_at_end(&list_mem,white->row,white->line);
                        break;
                    }
                }
            }
            else if(strrchr(string,'g')!=NULL || strrchr(string,'G')!=NULL){
                 ai_generate(&(white->row),&(white->line),&(black->row),&(black->line),list_mem,wall_ho,2*walls-table.walls_b,turn,table.size);
                 insert_at_end(&list_mem,white->row,white->line);
                 break;
            }
             fflush(stdout);
            }
        }    
        display_board(table.size,table.walls_w,table.walls_b,white->line,white->row,black->line,black->row,walls,wall_ho,wall_ve);
        turn++;
    }
    free(white); //free memory for pointers
    free(black);
    free(list_mem);//thelei douleia
    return 0;
}