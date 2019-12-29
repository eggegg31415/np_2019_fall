#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<dirent.h>
#include<errno.h>
#include<sys/stat.h>

int main(){
    //char name[] = "0";
    //DIR* dir = opendir(name);
    //struct dirent *file;
    //if(dir){
    //    while((file = readdir(dir)) != NULL){
    //        if(strncmp(file->d_name, ".", 1) == 0)
    //            continue;
    //        printf("%s\n", file->d_name);
    //    }
    //    closedir(dir);
    //}
    //else{
    //    mkdir(name, 0777);
    //}

    FILE *fp = fopen("test/test.c", "rb");
    FILE *dst = fopen("asdf/test.c", "wb");
    char msg[1024];
    int len;

    while(len = read(fileno(fp), msg, 1024)){
        //write(fileno(stdout), msg, len);
        write(fileno(dst), msg, len);
    }
}
