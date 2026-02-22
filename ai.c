#include <stdlib.h>
#include<stdio.h>
#include "header.h"

void ai_generate(int *row,int *line,int *op_row,int *op_line,listnode alist,int hor[][2],int count_walls,int turns,int size){
    
    int i,flag=1,counter=0;
    if(turns%2==1){ //when turn is odd blacks' player
        
    for(i=0; i<count_walls; i++){ //condition for ai to recognize if there are walls in front of it
        if(hor[i][1]-*line==1 || hor[i][0]-*line==0 &&(hor[i][0]==*row)){
            flag=0;
        }
    }
        if(turns==1 && flag==1)
            --(*line);
        else if(turns==1 && flag==0){
                    for(i=0; i<count_walls; i++)
                    {
                        if(*row>1 && *row<size){
                            if(hor[i][0]==*row){
                                --(*row); 
                                counter=1;
                            }   
                    }
                    else if(*row==1 && counter==0){
                        ++(*row);
                    }
                    else
                        --(*row);
                    }
        } 
        if(*line-1==1){  ////if piece is one square away from finish
            if(*op_line==*line-1 && *op_row==*row){ //if there is opponent piece in front of it
                    if(*row<=size/2)
                        ++(*row);
                    else
                        --(*row);
                }
                else if(flag==0){
                    for(i=0; i<count_walls; i++)
                    {
                        if(*row>1 && *row<size){
                            if(hor[i][0]==*row){
                                --(*row); 
                                counter=1;
                            }   
                    }
                    else if(*row==1 && counter==0){
                        ++(*row);
                    }
                    else
                        --(*row);
                    }
                }
                else
                    --(*line);
            }
            else if(*op_line==*line-1 && *op_row==*row && *op_line!=1 ){ //if there is opponent piece in front of it
                    if(flag==1)
                        *line=*line-2; //we jump over him
                    else{
                    for(i=0; i<count_walls; i++)
                    {
                        if(*row>1 && *row<size){
                            if(hor[i][0]==*row){
                                --(*row); 
                                counter=1;
                            }   
                    }
                    else if(*row==1 && counter==0){
                        ++(*row);
                    }
                    else
                        --(*row);
                    }
                    }

                }
        else if(size-*line!=*op_line-1 && turns!=1){ //if we are closer to finish compared to opponent
                    if(flag==1)
                        --(*line);
                    else
                    {
                    for(i=0; i<count_walls; i++)
                    {
                        if(*row>1 && *row<size){
                            if(hor[i][0]==*row){
                                --(*row); 
                                counter=1;
                            }   
                    }
                    else if(*row==1 && counter==0){
                        ++(*row);
                    }
                    else
                        --(*row);
                    }
                    }    
        }
        else if(*line-1==size-*op_line && turns!=1 && *op_line!=*line-1){ //if we are at the same distance from finish as our opponent
            if(*op_line==*line-2 && *op_row==*row){ //we don't want to give them the ability to jump over our piece
                if(*row<=size/2)
                        ++(*row);
                    else
                        --(*row);
            }
            else if(flag==0)
            {
                     for(i=0; i<count_walls; i++)
                    {
                        if(*row>1 && *row<size){
                            if(hor[i][0]==*row){
                                --(*row); 
                                counter=1;
                            }   
                    }
                    else if(*row==1 && counter==0){
                        ++(*row);
                    }
                    else
                        --(*row);
                    }
            }
            else if(flag==1)
                --(*line);
        }
            
    }

    else{ //its white's player turn
    for(i=0; i<count_walls; i++){ //condition for ai to recognize if there are walls in front of it
        if(hor[i][1]-*line==1 || hor[i][0]-*line==0 &&(hor[i][0]==*row)){
            flag=0;
        }
    } 
        if(turns==2 && flag==1)
            ++(*line);
        else if(turns==2 && flag==0){
                    for(i=0; i<count_walls; i++)
                    {
                        if(*row>1 && *row<size){
                            if(hor[i][0]==*row){
                                --(*row); 
                                counter=1;
                            }   
                    }
                    else if(*row==1 && counter==0){
                        ++(*row);
                    }
                    else
                        --(*row);
                    }
        } 
        if(*line==size-1){
            if(*op_line==*line+1 && *op_row==*row){ //if there is opponent piece in front of it
                if(*row<=size/2)
                    ++(*row);
                else
                    --(*row);
            }
                else if(flag==0){
                    for(i=0; i<count_walls; i++)
                    {
                        if(*row>1 && *row<size){
                            if(hor[i][0]==*row){
                                --(*row); 
                                counter=1;
                            }   
                    }
                    else if(*row==1 && counter==0){
                        ++(*row);
                    }
                    else
                        --(*row);
                    }
                }
            else
                ++(*line);
            }
            else if(*op_line==*line+1 && *op_row==*row && *op_line!=size){ //if there is opponent piece in front of it
                if(flag==1)
                    *line=*line+2; //we jump over him
                else{
                       for(i=0; i<count_walls; i++)
                    {
                        if(*row>1 && *row<size){
                            if(hor[i][0]==*row){
                                --(*row); 
                                counter=1;
                            }   
                    }
                    else if(*row==1 && counter==0){
                        ++(*row);
                    }
                    else
                        --(*row);
                    }
                }    
            }
        else if(size-*line!=*op_line-1 && turns!=2){  //if we are closer to finish compared to opponent
                if(flag==1)
                    ++(*line);
                else if(flag==0)
                {
                    for(i=0; i<count_walls; i++)
                    {
                        if(*row>1 && *row<size){
                            if(hor[i][0]==*row){
                                --(*row); 
                                counter=1;
                            }   
                    }
                    else if(*row==1 && counter==0){
                        ++(*row);
                    }
                    else
                        --(*row);
                    }
                }    
            }
        else if(size-*line==*op_line-1 && turns!=2 && *op_line!=*line+1){ //if we are at the same distance from finish as our opponent
            if(*op_line==*line+2 && *op_row==*row){ //we don't want to give them the ability to jump over our piece
                if(*row<=size/2)
                        ++(*row);
                    else
                        --(*row);
            }
            else if(flag==0)
            {
                    for(i=0; i<count_walls; i++)
                    {
                        if(*row>1 && *row<size){
                            if(hor[i][0]==*row){
                                --(*row); 
                                counter=1;
                            }   
                    }
                    else if(*row==1 && counter==0){
                        ++(*row);
                    }
                    else
                        --(*row);
                    }
            }
            else if(flag==1) 
                ++(*line);
        }
        }
        while (alist != NULL) {
            if((alist)->next==NULL) break;
        alist = ((alist)->next);    
    } 
} 