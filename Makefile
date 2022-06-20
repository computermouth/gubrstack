
all:
	gcc -Wall -std=gnu11 main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
