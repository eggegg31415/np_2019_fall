# Homework2 UDP Reliable File Transfer

## Overview
In this homework, you need to implement a UDP reliable file transfer protocol on the application level using 3 different timeout mechanisms.

The UDP reliable file transfer protocol includes a sender and a receiver. The sender keeps sending the content of a file to the receiver by using UDP packets. The receiver keeps receiving the content from the sender and outputs the received file content to another file. **Important: the maximum size of every UDP datagram (packet) transfered is limited to 1024 Bytes.**

To cope with the packet loss and re-ordering in the network, the sender and receiver should **detect the packet loss events and re-ordering events** (by using the timeout methods below) and deal with them (by re-transmitting lost packets).

Another important thing you should know is that **the available link bandwidth between the sender and the receiver will be limited** when we test your programs. Make sure that your retransmission mechanism works correctly under any link bandwidth.

Also, you have to **take RTT and queuing delay into consideration**. We will set RTT to a non-zero value and add variable queuing delays to packets when testing your programs.

## Specification
You should implement senders and receivers using 3 different timeout methods, respectively:

*   Timeout using SIGALRM
*   Timeout using select
*   Timeout using setsockopt

You can write as many source files as you need in this homework, and you can freely name your source files. However, after you compile your codes with Makefile, you have to make sure that **there will be 6 executables generated, and named as follows**:

* `sender_sigalrm`
* `receiver_sigalrm`
* `sender_select`
* `receiver_select`
* `sender_sockopt`
* `receiver_sockopt`

> sender - UDP client
> receiver - UDP server (you can reuse)

> Note: The receiver must NOT read a file locally to pretend that it has received the file from the sender.

You can output whatever information you need on the screen (`stdout`). We will only examine if the file sent and the file received are the same.

You can only use C or C++ in this homework.

### SIGALRM
`sender_sigalrm` will send a file to `receiver_sigalrm`, and `receiver_sigalrm` will save the file. Due to the unreliable underlying network conditions, your `sender_sigalrm` and `receiver_sigalrm` must use **SIGALRM** to implement your retransmission mechanism.

The command to start the receiver:

    receiver_sigalrm [save filename] [bind port]
    

The command to start the sender:

    sender_sigalrm [send filename] [target address] [connect port]
    

For example, if the sender is about to send a file named `a.txt` to the receiver, whose IP address and port are `127.0.0.1` and `5000`, and the receiver will save the file as `b.txt`:

*   Start the receiver first:
```
receiver_sigalrm b.txt 5000
```
*   Then start the sender:
```
sender_sigalrm a.txt 127.0.0.1 5000
```

*   After finishing the file transfer, **both `sender_sigalrm` and `receiver_sigalrm` should terminate automatically**.
*   We will test if `a.txt` is the same as `b.txt`.

> Note: For the **SIGALRM** timeout method, you need to use **siginterrupt** to allow the SIGALRM signal to interrupt a system call.

### select
`sender_select` will send a file to `receiver_select`, and `receiver_select` will save the file. Due to the unreliable underlying network conditions, your `sender_select` and `receiver_select` must use **select** to implement your retransmission mechanism.

The command to start the receiver:
```
receiver_select [save filename] [bind port]
```

The command to start the sender:
```
sender_select [send filename] [target address] [connect port]
```

For example, if the sender is about to send a file named `a.txt` to the receiver, whose IP address and port are `127.0.0.1` and `5000`, and the receiver will save the file as `b.txt`:

*   Start the receiver first:
```
receiver_select b.txt 5000
```

*   Then start the sender:
```
sender_select a.txt 127.0.0.1 5000
```

*   After finishing the transmission, **both `sender_select` and `receiver_select` should terminate automatically**.
*   We will test if `a.txt` is the same as `b.txt`.

### setsockopt
`sender_sockopt` will send a file to `receiver_sockopt`, and `receiver_sockopt` will save the file. Due to the unreliable underlying network conditions, your `sender_sockopt` and `receiver_sockopt` must use **setsockopt** to implement your retransmission mechanism.

The command to start the receiver:

    receiver_sockopt [save filename] [bind port]
    

The command to start the sender:

    sender_sockopt [send filename] [target address] [connect port]
    

For example, if the sender is about to send a file named `a.txt` to the receiver, whose IP address and port are `127.0.0.1` and `5000`, and the receiver will save the file as `b.txt`:

*   Start the receiver first:
```
receiver_sockopt b.txt 5000
```

*   Then start the sender:
```
sender_sockopt a.txt 127.0.0.1 5000
```

*   After finishing the transmission, you have to **both `sender_sockopt` and `receiver_sockopt` should terminate automatically**.
*   We will test if `a.txt` is the same as `b.txt`.


## Network Environment Settings
We will test if your program can work correctly under specific **bi-directional packet loss rates**, **a non-zero RTT**, and **a fixed link bandwidth**. We will randomly drop your packets and check whether the results are still correct.

To test your programs, you need to simulate the network scenarios (packet loss rate and packet re-ordering), and check if the file received is no different from the input.

Use the tool `tc` to simulate packet losses. `tc` is a linux built-in command line program to control the network traffic in the kernel. With `tc`, you can set network state parameters on a network interface card.

> You should **NOT** use `tc` on the workstation (i.e. inplinux, bsd, and linux). **Instead, you should do that on your own PC.**
> 
> Because it requires _administrator permission_ (i.e., `sudo`) to use `tc`, you can only use `tc` **on your local pc** to test if your senders and receivers work correctly when packet losses and re-ordering occur.
> 
> Of course, you still have to test your programs on the workstations to check if they can be built successfully with your Makefile, and check if they can work correctly in the ideal case (no packet loss, no link bandwidth limitation, and RTT is 0).

Refer to [man tc](http://man7.org/linux/man-pages/man8/tc.8.html) for more information.

### Add a rule to set the packet loss rate, queuing delay, link bandwidth, and re-ordering
In your linux shell, use the following command to set the **packet loss rate**, **queuing delay**, **link bandwidth**, and **re-ordering**:

    sudo tc qdisc add dev <Device> root netem loss random <Packet Loss Rate> delay <Delay Time> <Variation> distribution <Distribution> rate <Link Bandwidth> reorder <Percent>
    

For example, you can add a rule on egress to set the packet loss rate to **5%**, set the queuing delay to between **7 ms~13 ms** (i.e., 10ms ± 3ms) with normal distribution, set the link bandwidth to **200 kbps**, and set packet re-ordering probability to **25%** on device “lo”.

    sudo tc qdisc add dev lo root netem loss 5% delay 10ms 3ms distribution normal rate 200kbit reorder 25%
    

### Delete all rules on a device

In your linux shell, use the following command:

    sudo tc qdisc del dev <Device> root
    

For example, you can delete all rule settings on egress on the device “lo”.

    sudo tc qdisc del dev lo root
    

> Note: If there is an error message like `RTNETLINK answers: File exists` when you add a rule, try to delete all rules first.

### Display all rules on a device

In your linux shell, use the following command:
```
sudo tc qdisc show dev <Device>
```

For example, you can display all rule settings on the device “lo”.
```
sudo tc qdisc show dev lo
```

## Generate Test Files Randomly
You can use `dd` to generate a test file of a specific file size with random contents. `dd` is a linux built-in command line program used to convert and copy a file.

> Refer to [man dd](http://man7.org/linux/man-pages/man1/dd.1.html) for further information.

Type the following command in your linux shell to generate one file named `<Ouput File Name>` with the file size `<File Size>`:

    dd if=/dev/urandom of=<Output File Name> bs=<File Size> count=1
    

For example, the following command generates a file named `a_big_file` with file size = `5MB`:

    dd if=/dev/urandom of=a_big_file bs=5MB count=1
    

## Check Correctness
Use the tool `diff` to check if the file transferd is correct. `diff` is also a built-in linux command line tool.

In your linux shell, use the following command:
```
diff <File 1> <File 2>
```
For example, you can check if the file `a_file` is equal to `b_file`.
```
diff a_file b_file
```

If there is no message output on the screen, it means that `a_file` is equal to `b_file`, otherwise it will show the differences between `a_file` and `b_file`.
