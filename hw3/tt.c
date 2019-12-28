#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>

int main(){
    for(int i=1; i<10; i++){
        printf("\r");
        printf("progress : [");
        for(int j=0; j<i; j++)
            printf("#");
        printf("]\n");
        fflush(stdout);
        //sleep(1);
    }
    printf("\n");
}
