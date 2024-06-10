cc=clang
main=./src/main.c
out=./bin/main
DEBUG= -g #-fsanitize=address
OPTS= $(DEBUG)
	#-std=c++11
LIBS= -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

build:
	$(cc) $(main) -o $(out) $(LIBS) 

debug:
	$(cc) $(main) -o $(out) $(OPTS) $(LIBS)

run: build
	$(out)

test: debug
	$(out)

