all:
	gcc -std=gnu11 -O3 -o pingsub pingsub.c
	gcc -std=gnu11 -O3 -o get_ifi_info get_ifi_info.c
	gcc -std=gnu11 -O3 -o ipurl ipurl.c

test:
	gcc -std=gnu11 -O3 -o tt tt.c

clean:
	if [ -f get_ifi_info ];then rm get_ifi_info; fi
	if [ -f pingsub ];then rm pingsub; fi
	if [ -f ipurl ]; then rm ipurl; fi
