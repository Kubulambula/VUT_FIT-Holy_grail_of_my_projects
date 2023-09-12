# Janšta Jakub (xjanst02)


CC = gcc
CFALGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
FILES = src/proj2.c src/queue.c src/util.c
EXECUTABLE = proj2
ZIP = proj2.zip


default:
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(FILES) -pthread -lrt


zip:
	zip $(ZIP) src/*.h src/*.c Makefile
