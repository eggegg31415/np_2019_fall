all:
	gcc sender_select.c -o sender_select -std=gnu99 -O2
	gcc receiver_select.c -o receiver_select -std=gnu99 -O2
	gcc sender_sigalrm.c -o sender_sigalrm -std=gnu99 -O2
	gcc receiver_sigalrm.c -o receiver_sigalrm -std=gnu99 -O2
	gcc sender_sockopt.c -o sender_sockopt -std=gnu99 -O2
	gcc receiver_sockopt.c -o receiver_sockopt -std=gnu99 -O2
	touch out
	rm out

clean:
	rm sender_select
	rm receiver_select
	rm sender_sockopt
	rm receiver_sockopt
	rm sender_sigalrm
	rm receiver_sigalrm
