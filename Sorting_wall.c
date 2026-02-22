#include "header.h"

void sorting(int array[][2],int walls) {
    int temp1,temp2,i,j;
    for(i=0;i<=walls-1;i++) {
        for(j=i;j>0;j--){
            if(array[j][1]>array[j-1][1]) {
                temp1=array[j][0];
                temp2=array[j][1];
                array[j][0]=array[j-1][0];
                array[j][1]=array[j-1][1];
                array[j-1][0]=temp1;
                array[j-1][1]=temp2;
            }
            else if(array[j][1]==array[j-1][1]) {
                if(array[j][0]<array[j-1][0]) {
                    temp1=array[j][0];
                    array[j][0]=array[j-1][0];
                    array[j-1][0]=temp1;
                }
            }
        }
    }
}