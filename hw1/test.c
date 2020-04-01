// Test cases for INP homework 1
// Do not modify this file TA will replace this file with the original version.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>

#define RED(s) "\033[0;32;31m"s"\033[m"
#define GREEN(s) "\033[0;32;32m"s"\033[m"
#define TEST(name,score_of_test) void test_##name() {\
			printf("[RUN] %s, %d points\n", __func__, score_of_test);\
			current_score = score_of_test;\
			is_passed = 1;\
			setup(#name);
#define END_TEST(name) \
			if(is_passed) {\
				score.name = current_score;\
				printf(GREEN("    [PASS] %s\n"), __func__);\
			}\
			tear_down();}
#define ASSERT_EQ_STR(expected,actual)\
	if(strcmp(expected, actual) != 0) {\
		printf(RED("[FAIL] %s\n"), __func__);\
		printf("\tExpected: %s", expected);\
		printf("\tActual  : %s", actual);\
		is_passed = 0;\
	}
#define FAIL()\
	printf(RED("[FAIL] %s, at line %d\n"), __func__, __LINE__);\
	is_passed = 0;
#define MAX_BUFFER 256
#define NUM_CLIENTS 3 // At least 3 clients are required for these tests

//To add a test case, you must add a new field in "score", and then use 
//TEST, END_TEST to write your test code, and call the test case in main
int current_score = 0;
int is_passed = 1;
char expected[MAX_BUFFER];

struct score_t {
	int hello;
	int name;
	int who;
	int exit;
	int tell;
	int yell;
} score = {0};

typedef struct {
	pid_t pid;
	int input_fd, output_fd;
} child_proc;

void open_proc(child_proc* proc, char** args) {
	int pipe1[2], pipe2[2];
	if(pipe(pipe1) == -1 || pipe(pipe2) == -1) {
		perror("pipe");
		exit(0);
	}

	pid_t pid = fork();
	if(pid == 0) {
		close(pipe1[1]);
		close(pipe2[0]);
		dup2(pipe1[0], STDIN_FILENO);
		dup2(pipe2[1], STDOUT_FILENO);

		setenv("PATH", "", 1);
		if(execvp(args[0], args) < 0) {
			perror("exec error");
			exit(0);
		}
	} else if(pid == -1) {
		perror("fork");
		exit(0);
	}

	close(pipe1[0]);
	close(pipe2[1]);
	proc->input_fd = pipe1[1];
	proc->output_fd = pipe2[0];
	proc->pid = pid;
}

void close_proc(child_proc* proc) {
	close(proc->input_fd);
	close(proc->output_fd);
	kill(proc->pid, SIGTERM);
}

void set_proc_input(child_proc* proc, char* msg) {
	write(proc->input_fd, msg, strlen(msg));
}

//get a line from proc
void get_proc_output(child_proc* proc, char* buffer, size_t buffer_size) {
	buffer[0] = 0;
	int length = 0;
	fd_set read_set;
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = 1000000;

	while(1) {
		FD_ZERO(&read_set);
		FD_SET(proc->output_fd, &read_set);
		int ret = select(proc->output_fd+1, &read_set, NULL,NULL, &timeout);
		if(ret == -1) {
			printf(RED("    [ERROR] select failed\n"));
			break;
		} else if(ret == 0) {
			printf(RED("    [ERROR] read timeout\n"));
			break;
		} else {
			char c;
			read(proc->output_fd, &c, 1);
			buffer[length] = c;
			buffer[length+1] = 0;
			++length;
			if(c == '\n' || length > buffer_size-1) {
				break;
			}
		}
	}
}

child_proc server_proc;
child_proc client_procs[NUM_CLIENTS];
unsigned short port;
char buffer[MAX_BUFFER];
char* server_ip = "127.0.0.1";
char client1_ip_port[32];

void setup(char* test_name) {
	srand(time(NULL));
	port = rand()%20000 + 10000;

	char* args[5];
	args[0] = "./server";
	sprintf(buffer, "%d", port);
	args[1] = buffer;
	args[2] = NULL;
	open_proc(&server_proc, args);
	sleep(1);

	args[0] = "./client";
	args[1] = server_ip;
	sprintf(buffer, "%d", port);
	args[2] = buffer;
	args[3] = NULL;

	int i;
	for(i = 0; i < NUM_CLIENTS; ++i) {
		open_proc(&client_procs[i], args);
		usleep(200000); // wait for 0.2 second
	}

	//get the client's ip:port
	sprintf(buffer, "lsof -n -i tcp:%d | grep \" %d \"", port, client_procs[0].pid);
	FILE* fp = popen(buffer, "r");
	fgets(buffer, MAX_BUFFER, fp);
	char* target_end = strstr(buffer, "->");
	char* target_begin = strstr(buffer, "TCP");
	target_end[0] = '\0';
	target_begin += 4;
	sscanf(target_begin, "%s", client1_ip_port);
	pclose(fp);
}

void tear_down() {
	close_proc(&server_proc);

	int i;
	for(i = 0; i < NUM_CLIENTS; ++i)
		close_proc(&client_procs[i]);
}

void consume_all_hello_msg() {
	int i, j;
	for(i = 0; i < NUM_CLIENTS; ++i) {
		for(j = 0; j < NUM_CLIENTS-i; ++j)
			get_proc_output(&client_procs[i], buffer, MAX_BUFFER);
	}
}

TEST(hello, 10)
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] Hello, anonymous! From: %s\n", client1_ip_port);
	ASSERT_EQ_STR(expected, buffer);

	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] Someone is coming!\n");
	ASSERT_EQ_STR(expected, buffer);
END_TEST(hello)

TEST(who, 15)
	consume_all_hello_msg();

	//repeated calling of who should obtain identical result
	for(int i = 0; i < 3; ++i) {
		set_proc_input(&client_procs[0], "who\n");
		for(int i = 0; i < 2; ++i) {
			get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
			if(strstr(buffer, "->me") != NULL) {
				sprintf(expected, "[Server] anonymous %s ->me\n", client1_ip_port);
				ASSERT_EQ_STR(expected, buffer);
			}
		}
	}
END_TEST(who)

TEST(name, 15)
	consume_all_hello_msg();

	set_proc_input(&client_procs[0], "notcommand\n");
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);

	sprintf(expected, "[Server] ERROR: Error command.\n");
	ASSERT_EQ_STR(expected, buffer);

	set_proc_input(&client_procs[0], "name anonymous\n");
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] ERROR: Username cannot be anonymous.\n");
	ASSERT_EQ_STR(expected, buffer);

	set_proc_input(&client_procs[0], "name 112233\n");
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] ERROR: Username can only consists of 2~12 English letters.\n");
	ASSERT_EQ_STR(expected, buffer);

	set_proc_input(&client_procs[0], "name judy\n");
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] You're now known as %s.\n", "judy");
	ASSERT_EQ_STR(expected, buffer);

	set_proc_input(&client_procs[0], "who\n");
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] %s %s ->me\n", "judy", client1_ip_port);
	ASSERT_EQ_STR(expected, buffer);
END_TEST(name)

TEST(exit, 10)
	consume_all_hello_msg();

	set_proc_input(&client_procs[2], "exit\n");
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] anonymous is offline.\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[1], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] anonymous is offline.\n");
	ASSERT_EQ_STR(expected, buffer);
END_TEST(exit)

TEST(tell, 20)
	consume_all_hello_msg();

	set_proc_input(&client_procs[0], "name judy\n");
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] You're now known as judy.\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[1], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] anonymous is now known as judy.\n");
	ASSERT_EQ_STR(expected, buffer);

	set_proc_input(&client_procs[1], "name jeff\n");
	get_proc_output(&client_procs[1], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] You're now known as jeff.\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] anonymous is now known as jeff.\n");
	ASSERT_EQ_STR(expected, buffer);

	//client1 tell client1
	set_proc_input(&client_procs[0], "tell judy hi there\n");
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] SUCCESS: Your message has been sent.\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] judy tell you hi there\n");
	ASSERT_EQ_STR(expected, buffer);

	//client1 tell client 2
	set_proc_input(&client_procs[0], "tell jeff let's have fun!!\n");
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] SUCCESS: Your message has been sent.\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[1], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] judy tell you let's have fun!!\n");
	ASSERT_EQ_STR(expected, buffer);
END_TEST(tell)

TEST(yell, 20)
	consume_all_hello_msg();

	set_proc_input(&client_procs[0], "name judy\n");
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] You're now known as judy.\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[1], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] anonymous is now known as judy.\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[2], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] anonymous is now known as judy.\n");
	ASSERT_EQ_STR(expected, buffer);

	set_proc_input(&client_procs[1], "name jeff\n");
	get_proc_output(&client_procs[1], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] You're now known as jeff.\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] anonymous is now known as jeff.\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[2], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] anonymous is now known as jeff.\n");
	ASSERT_EQ_STR(expected, buffer);

	set_proc_input(&client_procs[2], "name alice\n");
	get_proc_output(&client_procs[2], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] You're now known as alice.\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] anonymous is now known as alice.\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[1], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] anonymous is now known as alice.\n");
	ASSERT_EQ_STR(expected, buffer);

	set_proc_input(&client_procs[0], "yell I am so awesome!\n");
	get_proc_output(&client_procs[0], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] judy yell I am so awesome!\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[1], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] judy yell I am so awesome!\n");
	ASSERT_EQ_STR(expected, buffer);
	get_proc_output(&client_procs[2], buffer, MAX_BUFFER);
	sprintf(expected, "[Server] judy yell I am so awesome!\n");
	ASSERT_EQ_STR(expected, buffer);
END_TEST(yell)

void summary() {
	int sum = 0;
	int i;
	for(i = 0; i < sizeof(score)/sizeof(int); ++i)
		sum += *((int*)&score + i);
	printf("Score: %d, the demo accounts for the remained 10 points.\n", sum);
}

void int_handler(int signum) {
	close_proc(&server_proc);

	int i;
	for(i = 0; i < NUM_CLIENTS; ++i)
		close_proc(&client_procs[i]);
	summary();
	exit(0);
}

int main(int argc, char** argv) {
	signal(SIGINT, int_handler);

	test_hello();
	test_who();
	test_name();
	test_exit();
	test_tell();
	test_yell();
	
	summary();
	return 0;
}
