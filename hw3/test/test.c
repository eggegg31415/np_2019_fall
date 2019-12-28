#include<stdio.h>
#include <fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include<string.h>
#include<signal.h>

#define PAUSE printf("Press Enter key to continue..."); fgetc(stdin);

/* global var */
int online = 0,serverUP = 0;
int blackhole;
pid_t id[10],server;
int pipe_in[10][2];
int pipe_out[10][2];

/* copy the flie to the des */
void cp(char*,char*);
/* send to client */
void sendSTDIN(int[],char*);
/* open a client */
void openClient(pid_t*,int[],int[],int,char*,char*,char*);
/* do diff a b*/
void diff_fork(int[],char*,char*);
void diff(char*,char*,char*,char*,int*);
void openServer(pid_t*,char*);
void intHandler(int);
int printMessage = 0;
int main(int argc,char *argv[]){	
	int score_onefile = 20;
	int score_exit = 10;
	int score_login = 15;
	int score_same = 15;
    int score_user = 10;
	int score_sleep = 30;

	char buf[10000];
	char sysbuf[100];
	char sysbuf2[100];	
	int i,n;

	signal(SIGINT, intHandler);

	if(argc != 4){
		printf("Error: ./test <path to program's directory> <port> <1 to print server/clinet message,0 don't print>\n");
		return 0;
	}

    if((blackhole = open("/dev/null",O_WRONLY)) == -1)
        perror("blackhole");
	/* save cwd */
	char cwd[1000];
	getcwd(cwd, sizeof(cwd));
	printMessage = atoi(argv[3]);
	if(chdir(argv[1]) == -1){
		printf("Error: Can't change dir.\n");
		return 0;
	}
	
	/* make */
	system("make");
	system("clear");
	printf("Test : Start Homework3 test.\n\n\n");
	
	/* open server */	
	openServer(&server,argv[2]);	

	/********************* Test1 *********************/
        /* Two clients , client 0 upload a file , client 1 recieve file*/
	printf("Test1 : upload a file.\n");
	printf("\tTest1 : open Client0 Client1.\n");

	/* open two clients */
	for(i = 0; i < 2 ; i++){
                openClient(&id[i],pipe_in[i],pipe_out[i],i,"127.0.0.1",argv[2],"tom");
        }
	sleep(2);

	/* copy the testfile0 to client0 */
	sprintf(sysbuf,"%s/testfile",cwd);
	cp(sysbuf,"0/");
	
	/* client 0 : put testfile */
	printf("\tTest1 : Client0: put testfile.\n");
	sendSTDIN(pipe_in[0],"put testfile\n");
	/* wait for client upload/download file */	
	sleep(10);

	printf("\t--------------------\n");
	/* diff */
	diff("0/testfile","1/testfile","\tTest1 : upload a file PASS.\n","\tTest1 : upload a file FAIL.\n",&score_onefile);
	printf("\n");
	/********************* Test1 finish *********************/

	
	/********************* Test2 *********************/
	/* A user login and get file which client0 send */
	printf("Test2 : a user login and get file.\n");
	printf("\tTest2 : open Client2.\n");

	/* open one client */
	openClient(&id[2],pipe_in[2],pipe_out[2],2,"127.0.0.1",argv[2],"tom");
	
	/* wait for client sync file */	
    sleep(8);
	
	printf("\t--------------------\n");

	/* diff */
	diff("0/testfile","2/testfile","\tTest2 : login and get data PASS.\n","\tTest2 : login and get data FAIL.\n",&score_login);
	printf("\n");
 	/********************* Test2 finish *********************/
	
	/********************* Test3 *********************/
	/* Two clients upload at the same time */
	printf("Test3 : upload at the same time.\n");

	/* copy file */
	sprintf(sysbuf,"cp %s/testfile2 0/testfile2",cwd);  
	system(sysbuf);
	sprintf(sysbuf,"cp %s/testfile3 1/testfile3",cwd);
        system(sysbuf);
	
	/* put file at the same time*/
	printf("\tTest3 : Client0: put testfile2.\n");
	printf("\tTest3 : Client1: put testfile3.\n");
	sendSTDIN(pipe_in[0],"put testfile2\n");
	sendSTDIN(pipe_in[1],"put testfile3\n");

	sleep(20);
	printf("\t--------------------\n");

	/* diff */
	diff("0/testfile2","1/testfile2","\tTest3 : same time upload PASS1.\n","\tTest3 : same time upload FAIL1.\n",&score_same);	
	diff("0/testfile2","2/testfile2","\tTest3 : same time upload PASS2.\n","\tTest3 : same time upload FAIL2.\n",&score_same);	
	diff("1/testfile3","2/testfile3","\tTest3 : same time upload PASS3.\n","\tTest3 : same time upload FAIL3.\n",&score_same);
	diff("1/testfile3","0/testfile3","\tTest3 : same time upload PASS4.\n","\tTest3 : same time upload FAIL4.\n",&score_same);
	printf("\n");
	/********************* Test3 finish *********************/

	/********************* Test4 *********************/
	/* when some one sleep ,server non-blocking */
	printf("Test4 : sleep and non-blocking.\n");
	printf("\tTest4 : open Client3.\n");

	/* copy file */
	sprintf(sysbuf,"cp %s/testfile4 0/testfile4",cwd);
        system(sysbuf);
	
	/* open client */
	openClient(&id[3],pipe_in[3],pipe_out[3],3,"127.0.0.1",argv[2],"tom");
	/* while client sync data we will send sleep cmd after 2 seconds */
    sleep(2);
	
	/* client3 sleep , client 0 put*/
	printf("\tTest4 : Client3: sleep 20\n");
	printf("\tTest4 : Client0: put testfile4\n");
	sendSTDIN(pipe_in[3],"sleep 20\n");
	sendSTDIN(pipe_in[0],"put testfile4\n");
	sleep(20);

	printf("\t--------------------\n");

	/* diff */
	diff("0/testfile4","1/testfile4","\tTest4 : Sleep PASS1.\n","\tTest4 : Sleep FAIL1.\n",&score_sleep);	
	diff("0/testfile4","2/testfile4","\tTest4 : Sleep PASS2.\n","\tTest4 : Sleep FAIL2.\n",&score_sleep);	
	sleep(10);
	diff("0/testfile4","3/testfile4","\tTest4 : Sleep PASS3.\n","\tTest4 : Sleep FAIL3.\n",&score_sleep);
	printf("\n");
	/********************* Test4 finish *********************/

	/********************* Test5 user *********************/
	/* when some one sleep ,server non-blocking */
	printf("Test5 : login different user.\n");
	printf("\tTest5 : open Client4.\n");

	/* copy file */
	sprintf(sysbuf,"cp %s/testfile5 0/testfile5",cwd);
        system(sysbuf);
	
	/* open client */
	openClient(&id[4],pipe_in[4],pipe_out[4],4,"127.0.0.1",argv[2],"frank");
	sleep(2);
	
	printf("\tTest5 : Client0: put testfile5\n");
	sendSTDIN(pipe_in[0],"put testfile5\n");
	sleep(3);

	printf("\t--------------------\n");

	/* diff */
	diff("0/testfile5","1/testfile5","\tTest5 : User PASS1.\n","\tTest5 : User FAIL1.\n",&score_user);	
	diff("0/testfile5","2/testfile5","\tTest5 : User PASS2.\n","\tTest5 : User FAIL2.\n",&score_user);	
	diff("0/testfile5","3/testfile5","\tTest5 : User PASS3.\n","\tTest5 : User FAIL3.\n",&score_user);
    if( system("[ -e 4/testfile5 ]")	== 0)
    {
        printf("\tTest5: Tranfer to user4\n");
        score_user = 0;
    }
    printf("\n");


	/********************* Test6 Exit *********************/
	printf("Test6 : Exit, and delete all file.\n");
	PAUSE
	int status;
	for(i = 0; i < 5 ; i++){
		sendSTDIN(pipe_in[i],"exit\n");
		printf("pid[%d] = %d ",i,id[i]);
		sleep(1);
		if ((waitpid(id[i], &status, WNOHANG)) == 0){
			/* Exit fail */
			printf("exit FAIL.\n");
			kill(id[i], SIGKILL);
			score_exit = 0;
		}else{
			printf("exit PASS.\n");
		}
		sprintf(sysbuf,"rm -rf %d",i);
		system(sysbuf);
	}
	kill(server, SIGKILL);
	sleep(1);

	printf("\nTest1 : %d\n",score_onefile);
	printf("Test2 : %d\n",score_login);
	printf("Test3 : %d\n",score_same);
	printf("Test4 : %d\n",score_sleep);
	printf("Test5 : %d\n",score_user);
	printf("Test6 : %d\n\n",score_exit);
	printf("\t--------------------\n");
	printf("Total : %d\n",score_onefile+score_login+score_same+score_sleep+score_user+score_exit);
    close(blackhole);
}
void cp(char *filename,char *des){
        char buf[100];
        /* cp filename des */
        sprintf(buf,"cp %s %s",filename,des);
        system(buf);
}
void sendSTDIN(int fd[],char *ptr){
        char buf[9999];
        strcpy(buf,ptr);
        write(fd[1],buf,strlen(buf));
}
void openClient(pid_t *id,int STDIN[],int STDOUT[],int dirInt,char *ip,char *port,char *name){
        char buf[50],*arg[5];
        char dir[50];
        int i;
        sprintf(dir,"%d",dirInt);

        /* mkdir ptr */
        sprintf(buf,"mkdir %s",dir);
        system(buf);

        /* cp client ptr */
        cp("client",dir);

        if(pipe(STDIN)){
                perror("Pipe OpenClient");
                exit(0);
        }

        if(pipe(STDOUT)){
                perror("Pipe OpenClient");
                exit(0);
        }

        switch(*id = fork()){
                case -1:
                        perror("fork");
                        exit(0);
                case 0:
                        /* child */
                        /* stdin from parent */
                        close(STDIN[1]);
                        dup2(STDIN[0],STDIN_FILENO);
                        close(STDIN[0]);

                        /* stdout to parent */
                        close(STDOUT[0]);
                        if(printMessage < 1)
                        {
                            dup2(blackhole,STDOUT_FILENO);
                            dup2(blackhole,STDERR_FILENO);
                        }
                        close(blackhole);
                        close(STDOUT[1]);
                        chdir(dir);
                        setenv("PATH","",1);
                        arg[0] = "client";
                        arg[1] = ip;
                        arg[2] = port;
                        arg[3] = name;
                        arg[4] = NULL;
//                      printf("Dir :%s",dir);
//                      for(i = 0; i < 5 ; i++) printf("arg[%d] = %s\n",i,arg[i]);
                        if(execvp(arg[0],arg) < 0){
                                perror("exec");
                                exit(0);
                        }
                        break;
        }
        printf("Client[%d]  pid is %d\n", dirInt, *id);
        online += 1;
        close(STDIN[0]);
        close(STDOUT[1]);

}
void diff_fork(int STDOUT[],char *a,char *b){
        char *arg[4];

        if(pipe(STDOUT)){
                perror("Pipe");
                exit(0);
        }

        switch(fork()){
                case -1:
                        perror("fork");
                        exit(0);
                case 0:
                        /* child */
                        /* stdout to parent */
                        sleep(1);
                        close(STDOUT[0]);
                        dup2(STDOUT[1],STDOUT_FILENO);
                        dup2(STDOUT[1],STDERR_FILENO);
                        close(STDOUT[1]);

                        setenv("PATH","/usr/bin",1);
                        arg[0] = "diff";
                        arg[1] = a;
                        arg[2] = b;
                        arg[3] = NULL;
                        if(execvp(arg[0],arg) < 0){
                                perror("exec");
                                exit(0);
                        }
                        break;
        }
        int retval = fcntl(STDOUT[0], F_SETFL, fcntl(STDOUT[0], F_GETFL) | O_NONBLOCK);
//      printf("Ret from fcntl: %d\n", retval);
}

void diff(char *fileA,char *fileB,char *P,char *F,int *result){
        char buf[10000];
        int pipe_diff[2];
        diff_fork(pipe_diff,fileA,fileB);
        sleep(5);
        int n = read(pipe_diff[0],buf,sizeof buf);
        if(n == -1){
                printf("%s",P);
        }else{
                printf("%s",F);
                *result = 0;
                printf("\t%s\n",buf);
        }
        close(pipe_diff[0]);
        close(pipe_diff[1]);
}

void openServer(pid_t *server,char *port){
        /* open server */
        char *arg[5];

        switch(*server = fork()){
                case -1:
                        perror("fork");
                        exit(0);
                case 0:
                        if(printMessage < 2)
                        {
                            dup2(blackhole,STDOUT_FILENO);
                            dup2(blackhole,STDERR_FILENO);
                        }
                        close(blackhole);
                        setenv("PATH","",1);
                        arg[0] = "server";
                        arg[1] = port;
                        arg[2] = NULL;
                        if(execvp(arg[0],arg) < 0){
                                perror("exec server");
                                exit(0);
                        }
                        break;
        }
        serverUP = 1;
        sleep(2);
}

void intHandler(int a){
        int status;
        int i;
        char sysbuf[1000],sysbuf2[1000];
        printf("\n");
        for(i = 0; i < online ; i++){
                printf("client%d pid[%d] = %d ",i,i,id[i]);
                kill(id[i], SIGKILL);
                printf("exit.\n");
                sprintf(sysbuf,"rm -rf %d",i);
                printf("%s\n",sysbuf);
                system(sysbuf);
        }
        if(serverUP == 1){
                printf("\nkill server.\n");
                kill(server, SIGKILL);
        }
        exit(0);
}

