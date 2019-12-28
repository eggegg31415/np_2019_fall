#include<stdio.h>
#include<sys/types.h>
#include<sys/time.h>
#define max(a, b) a>b?a:b
int main(){
    struct timeval times;
    gettimeofday(&times, 0);
    printf("%d\n", times.tv_sec);
    sleep(10);
    gettimeofday(&times, 0);
    printf("%d\n", times.tv_sec);
}
