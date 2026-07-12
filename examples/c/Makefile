all:
	$(CC) -o example.bin example.c ../../src/crossdb.c -O2 -I../../include -lpthread
	./example.bin

debug:
	$(CC) -o example.bin example.c ../../src/crossdb.c -lpthread -g -fsanitize=address
	./example.bin

gdb:
	$(CC) -o example.bin example.c ../../src/crossdb.c -lpthread -g -fsanitize=address
	gdb example.bin

clean:
	rm -f a.out *.bin