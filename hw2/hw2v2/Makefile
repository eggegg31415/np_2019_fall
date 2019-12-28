all:
	gcc sender_select.c -o sender_select -std=gnu99
	gcc receiver_select.c -o receiver_select -std=gnu99
	gcc sender_sigalrm.c -o sender_sigalrm -std=gnu99
	gcc receiver_sigalrm.c -o receiver_sigalrm -std=gnu99
	gcc sender_sockopt.c -o sender_sockopt -std=gnu99
	gcc receiver_sockopt.c -o receiver_sockopt -std=gnu99
	touch out
	rm out
test:
	gcc -std=gnu99 tt.c
	./a.out
jizz:
	gcc -std=gnu99 jizz.c
	./a.out > in
