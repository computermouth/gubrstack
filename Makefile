
all:
	gcc -Wall -std=gnu11 main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

arm:
	gcc -Wall -std=gnu11 main.c -lraylib -lGLESv2 -ldrm -lgbm -lm -lpthread -ldl -lrt
