#include<stdio.h>
#include<sys/time.h>

int main(){
    struct timeval time;
    gettimeofday(&time, NULL);
    int i = time.tv_sec;
    printf("%d\n", i);
}

