# Homework 1 Specification

## Overview

In this homework, you are required to write two programs, a client and a server, to build a chat room system. The clients talk to one another through the server. The server accepts connections from the clients and processes the command from the clients.

:::info
**Hint:** 
It is suggested that you use **select()** for constructing the client and the server programs.
:::

## Specification
### Server

*   The command to start the server  
    `./server <SERVER PORT>`

If the number of arguments is not one, the server program should terminates.
The server can serve multiple clients simultaneously. Once a connection is established, the server will send a hello message to the client and give the client a username `anonymous`. The client can send different commands to the server. The messages transmitted between clients and the server are shown below.

#### Hello Message
When a client connects to the server, the server will send a hello message to the client and then broadcasts this user’s presence to other clients.

*   Client output format
    *   To the newly connected client
        `[Server] Hello, anonymous! From: <Client IP>:<Client Port>`
    *   To existing clients
        `[Server] Someone is coming!`
---
#### Offline Message
When a client disconnect from the server, the server will send an offline message to all the other online clients to tell them someone has been offline.

* Client output format
    `[Server] <USERNAME> is offline.`
---
#### Who Message
A client can type command below to list all online users.
* Client input format
    `who`

The server will reply to sender a list of online users and tag the sender client. For N user, Server will send N lines. Each of them shows details of a user.

* Client output format
    * If the user is not the client itself
        `[Server] <USERNAME> <CLIENT IP>:<CLIENT PORT>`
    * If the user is the client itself
        `[Server] <SENDER USERNAME> <CLIENT IP>:<CLIENT PORT> ->me`

* * *

#### Change Username Message
A client can type the command below to change his/her username.
* Client input format
    `name <NEW USERNAME>`
The server has to verify if the new name is valid which means the input name is
1.  not anonymous,
2.  unique, and
3.  **2~12** English letters.

It will reject user’s request if this name cannot fit the rule.

*   Client output format
    *   If the new name is anonymous.
        `[Server] ERROR: Username cannot be anonymous.`
        
    *   If the new name is not unique.
        `[Server] ERROR: <NEW USERNAME> has been used by others.`
        
    *   If the new name does not consist of **2~12** English letters.
        `[Server] ERROR: Username can only consists of 2~12 English letters.`
        

The server will reply some messages to all users once a user changes his/her name.

*   Client output format
    *   To user which changed his/her name
        `[Server] You're now known as <NEW USERNAME>.`
    *   To other users
        `[Server] <OLD USERNAME> is now known as <NEW USERNAME>.`
:::info
**Note:**
A user can be rename as itself, that is, when **userA** wants to rename as **userA**, server should not return error messages.
:::

---

#### Private Message

A client can send a private message to a specific client.
* Client input format
    `tell <USERNAME> <MESSAGE>`

The server will send an error message back to the sender if either the sender’s name or the receiver’s name is anonymous.

* Client output format
    * If the sender’s name is anonymous
        `[Server] ERROR: You are anonymous.`
    
    * If the receiver’s name is anonymous
        `[Server] ERROR: The client to which you sent is anonymous.`
        
    * If the receiver doesn’t exist
        `[Server] ERROR: The receiver doesn't exist.`
        

Otherwise, the server sends the private message to the specific client and sends back a notification to the sender.

*   Client output format
    *   To the sender: If the message is sent
        `[Server] SUCCESS: Your message has been sent.`
        
    *   To the receiver: If both client’s name are not anonymous
        `[Server] <SENDER USERNAME> tell you <MESSAGE>`
---
#### Broadcast Message

A client can send a broadcast message to all clients.

*   Client input format
    `yell <MESSAGE>`

While receiving the command from a user, the server will add `<SENDER's USERNAME>` at the head of it and broadcasts to all users including the sender.

*   Client output format
    `[Server] <SENDER's USERNAME> yell <MESSAGE>`
---
#### Error Command

Commands which haven’t been declared above are invalid commands. When a server receives an invalid command, it should send an error message back to the sending client.

*   Client output format
    `[Server] ERROR: Error command.`
---
### [](#Client "Client")Client

A client cannot connect to more than one server at the same time.
Users should give the server IP and the port as the arguments of the client program.
If the number of arguments is not two, the client program should terminates.

*   The command to start the client
    `./client <SERVER IP> <SERVER PORT>`
---

#### Exit Command
The user can type the command below to terminate the process at any time.

*   Command Format
    `exit`
:::info
**Note:**
This command should be process by the client locally. That is, the client should close the connection and terminate the process while it receives the **exit** command.
:::

---
#### [](#Receive-amp-Display-Format "Receive-amp-Display-Format")Receive & Display Format

The clients keep receiving commands from stdin and, except for “exit” command, send those to the server directly without any modification.

:::info
**Note:**
For messages received from stdin, client can only process the exit command, others should be sent to the server without modification. All commands received from stdin (except exit command) should be sent to server directly.
:::

## Requirement
1.  All messages transmitted between the server and the clients should end with a newline ('\\n').
2.  The clients should send commands received from stdin to server directly without modification.
3.  The server/client should NOT crash or be hanged.
4.  Use a Makefile to compile your code. The file names of the client and the server should be **client** and **server** respectively.
5.  There will be no more than **1024** characters in a single line.
6.  There will be no more than **10** clients connecting to a server in one time.
7.  All the code should be successfully compiled and run on the workstations.

## Grading Policy
### Features
* hello: 10%
* exit / offline message: 10%
* who: 15%
* name: 15%
* tell: 20%
* yell: 20%
* Demo: 10%

:::info
**Note:**
Each feature should be totally correct so that you can get full score for that feature. Take the `who` command for example, if you implement the function to change users’ name but don’t follow the output format, you will still loss the 15 points.
:::

### Penalty

*   Incorrect upload format (wrong directory structures, wrong file names etc.): -10%
*   Fail to `make` or `make test`: -10%
*   Late submission: final score = original score \* (3/4) ^ #\_late\_days
*   Cannot compile or run on the workstations: -20%

### Testing Flow

*   A test program is provided by TA. You should make sure your program is able to pass all the test cases.
*   The `test.c` file provided by TA should be included in your submission. You can modify it, but we will replace it with the original version while scoring.
*   We will use the following command to test your program.
```c++=
make
make test
```
*   Sample Makefile
    You should make sure the `make` command will generate both client and server binaries, and the `make test` command will generate the test binary and run the tests.
```c++=
# you can modify this section
all:
    gcc -o client client.c
    gcc -o server server.c
    
# test commands, you should not modify this section
test: all
    gcc -o test test.c
    stdbuf -o0 ./test
    
# you can modify this section
clean:
    rm -f server client test
```
## Submission
* Due date: **2019/11/7 Thu. 23:59**
* Hand-in format: **\<Student ID\>.zip**
* You must zip your code, or we won’t accept your submission. Do not use **rar**, **7z** or others to compress your code.
* Please make sure the `Makefile` is in the top level of the zip file (not be put in any folder). The test code provided by TA should be included in your submission, too.
*   It’s not necessary to upload your binaries as well since we will recompile your code from the sources.
*   The `test.c` and `Makefile` are the only two files that must be in your submission. Feel free to add your own header files or set the file name yourselves. Remember to modify the `Makefile` to fit it into your program structure.
*   Sample directory structure before `make`.
```
    /
    ├── client.c
    ├── server.c
    ├── test.c        # test program provided by TA
    └── Makefile      # your makefile, please refer to the sample provided by TA
```
*   Sample directory structure after `make && make test`
```
    /
    ├── client.c
    ├── server.c
    ├── test.c        # test program provided by TA
    ├── client        # client binary, must be generated by 'make' command
    ├── server        # server binary, must be generated by 'make' command
    ├── test          # test binary, must be generated by 'make test' command, must be in the same directory as server and client
    └── Makefile      # your makefile, please refer to the sample provided by TA
```


# Demo
The demo will be announced later. You will be asked to explain your implementation and make some changes to your program.  
**Please remember to select your demo time after we announce!**  
If your name doesn’t exist on our demo form, you won’t be allowed to demo your homework.
