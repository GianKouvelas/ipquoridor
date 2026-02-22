#include "header.h"

int playwall(char array[],int walls[][2],int *count_walls,int dimen) {
    int temp1,temp2,count=0;
    if(array[0]>90)
        temp1=array[0]-96;
    else if(array[0]<=90 )
        temp1=array[0]-64;  
    temp2=array[1]-48;
    if(dimen==1) {
        for(int i=0;i<(*count_walls);i++) {
            if(walls[i][1]==temp2) {
                count++;
                if(count==2) 
                    return 1;
            }
            else 
                count=0;
        }     
        walls[*count_walls][0]=temp1;
        walls[*count_walls][1]=temp2;
        (*count_walls)++;
    }
    else if(dimen==0) {
        for(int i=0;i<(*count_walls)*2;i++){
            if(walls[i][0]==temp1) {
                count++;
                if(count==4)
                    return 1;
            }
            else
                count=0;
        }
        walls[(*count_walls)][0]=temp1;
        walls[(*count_walls)][1]=temp2;
        (*count_walls)++;
        walls[(*count_walls)][0]=temp1;
        walls[(*count_walls)][1]=temp2-1;
        (*count_walls)++;
    }
    return 0;
}