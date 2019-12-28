#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>

int main(){
    //for(int i=1; i<10; i++){
    //    printf("\r");
    //    printf("progress:");
    //    for(int j=0; j<i; j++)
    //        printf("#");
    //    fflush(stdout);
    //    sleep(1);
    //}
    //printf("\n");

    char msg[1024];
    FILE *fp = fopen("Makefile", "rb");
    int filesize, len;
    filesize = ftell(fp);
    printf("%d %d\n", filesize, fileno(fp));

    while((len = read(fileno(fp), msg, sizeof(msg))) != 0){
        printf("%s", msg);
    }

    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    printf("%d\n", filesize);

    //fclose(fp);
    //fp = fopen("Makefile", "rb");
    rewind(fp);
    printf("%d %d\n", filesize, fileno(fp));
    while((len = read(fileno(fp), msg, sizeof(msg))) != 0){
        printf("%s", msg);
    }
}
