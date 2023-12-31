CFLAGS = -Wall
CC = gcc
BIN = edit

$(BIN): launch.c edit.c state.c text.c selection.c config.c
	$(CC) $(CFLAGS) -o $@ $^

install:	# as root
	chown root:root $(BIN)
	cp $(BIN) /bin

test: $(BIN)
	valgrind --leak-check=yes ./$(BIN) test.txt

clean:
	rm -f *.o $(BIN)
