all:
	gcc -O0 -g -std=gnu11 -o server server.c
	gcc -O0 -g -std=gnu11 -o client client.c
	if [ -d "tom" ]; then rm -rf tom; fi
	if [ -d "frank" ]; then rm -rf frank; fi

tt:
	gcc tt.c -std=gnu11
	./a.out

clean:
	rm server
	rm client
