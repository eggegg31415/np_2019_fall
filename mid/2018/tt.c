#include<stdio.h>
#include<string.h>

int main(){
    char in[1000];
    int n;
    while(1){
        char msg[1000];
        n = read(0, in, sizeof(in));
        in[n] = 0;
        char *token = strtok(in, " ");
        printf("%s\n", token);
        fflush(stdout);
    }
}
