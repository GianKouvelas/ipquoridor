#include <stdio.h>
#include <stdlib.h>
#include "header.h"

int playmove(char array[],int *row,int *line,listnode alist,int hor[][2],int ver[][2],int count_walls) {
    int i,temp,temp2,way1,way2; //way1 refers to row and way2 refers to line,and for example if the way1=1 it means that we changed row,but we are at the same line
        if(array[0]>90)
            temp=array[0]-96;
        else if(array[0]<=90 )
            temp=array[0]-64;  
        temp2=array[1]-48;
        for(i=0;i<count_walls;i++) {
            //we check if there is a wall in the way the player is moving
            //here we check for the horizontal walls
            if(((hor[i][1]-*line==1 && hor[i][1]-temp2==0) || (hor[i][1]-*line==0 && hor[i][1]-temp2==1)) && (temp==hor[i][0] || temp==hor[i][0]+1)) {
                printf("\nSorry wrong move,try again \n");
                return 1;
            }
        }
        for(i=0;i<count_walls*2;i++) {
            //we check if there is a wall in the way the player is moving
            //here we chenk for the vertical walls
            if(((ver[i][0]-*row==-1 && ver[i][0]-temp==0) || (ver[i][0]-*row==0 && ver[i][0]-temp==-1))  && ver[i][1]==temp2) {
                printf("\nSorry wrong move,try again \n");
                return 1;
            }
        }
        if(temp>*row + 1 || temp<*row -1 )
        {
            printf("\nSorry wrong move,try again \n");
            return 1;
        }   
        if (temp==*row+1 && (temp2==*line+1 || temp2==*line-1)) {
            printf("\nSorry wrong move,try again \n");
            return 1;
        }
        if (temp==*row-1 && (temp2==*line+1 || temp2==*line-1)) {
            printf("\nSorry wrong move,try again \n");
            return 1;
        }
        if(temp2>*line +1 || temp2<*line -1 )
        {
            printf("\nSorry wrong move,try again \n");
            return 1;
        }   
        if(temp==*row && temp2==*line)
        {
            printf("\nSorry wrong move,try again \n");
            return 1;
        }   
    if(array[0]>90) { //Moves are not case sensitive
        way1=array[0]-*row-96;
        *row=array[0]-96; //we subtract 96 because the character has a value of =<97 from ascii
    }
    else {
        way1=array[0]-*row-64;
        *row=array[0]-64;   //we subtract 64 because the character has a value of =<65 from ascii  
    }
    way2=array[1]-*line-48;
    *line=array[1]-48; //we subtract 48 because the character has a value of =<49 from ascii
    while (alist != NULL) {
        if((alist)->next==NULL) break;
        alist = ((alist)->next);
    } 
    if (alist->cord1==*row && alist->cord2==*line) { //it jumps over the other player
        if (way1==1)
            (*row)++;
        else if (way2==1)
            (*line)++;
        else if (way1==-1)
            (*row)--;
        else if (way2==-1)
            (*line)--;
    }
    return 0;
}