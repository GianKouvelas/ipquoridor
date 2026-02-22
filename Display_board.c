#include <stdio.h>
#include "header.h"

void display_board(int size,int walls_w,int walls_b,int linew,int roww, int lineb, int rowb,int walls,int wall_ho[][2],int wall_ve[][2]) {
    char ch;
    int i, i1, i2, j,table_line=size;
    printf("\n");
    i1=0;
    i2=0;
    for (j=1; j<=size*2+3 ; j++) {
        printf(" ");
        if(j==1 || j==size*2+3) {//when we are at the start or the end of the board we printf the characters
            printf("    ");
            ch='A';
            for (i=1; i <=size ; i++) {
                printf("%c   ",ch);
                ch++;
            }
            printf("  ");
        }
        else if (j%2==0) {
            printf("  +");
            for (i=1; i<=size ; i++) {
                    if((walls_b<walls || walls_w<walls) && wall_ho[i1][0]==i && table_line+1==wall_ho[i1][1]) {
                        printf("=======+");
                        i1++;
                        i++;
                    }
                    else
                        printf("---+");
            }
        }
        else {
            printf("%d |",table_line);
                for (i=1; i<=size ; i++) {
                    if (table_line==lineb && i==rowb) { 
                        printf(" B");
                        if((walls_b<walls || walls_w<walls) && wall_ve[i2][0]==i && (wall_ve[i2][1]==table_line)) {
                            printf(" H");
                            i2++;
                        }
                        else    
                            printf(" |");
                    }
                    else if (table_line==linew && i==roww) {
                        printf(" W");
                        if((walls_b<walls || walls_w<walls) && wall_ve[i2][0]==i && (wall_ve[i2][1]==table_line)) {
                            printf(" H");
                            i2++;
                        }
                        else    
                            printf(" |");
                    }
                    else {
                        if((walls_b<walls || walls_w<walls) && wall_ve[i2][0]==i && (wall_ve[i2][1]==table_line)) {
                            printf("   H");
                            i2++;
                        }
                        else    
                            printf("   |");
                    }
                }
            printf(" %d",table_line);
            if (table_line==size) { //when its the largest number we print the walls of the first player
                printf(" black walls:  %d",walls_b);
            }
            else if(table_line==size-1) {//when its the second largest number we printf the walls of the second player 
                printf(" white walls:  %d",walls_w);
            }
            table_line--;
        }
        printf("\n");
    }
}