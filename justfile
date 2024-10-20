flags := "-Wall -Wpedantic -g -ggdb -lm"
raylibFlags := "-I./include/raylib/ -I./include/ -L./include/raylib/ -lraylib"

default: make_build_dir
	gcc ./src/*.[ch] -o ./build/puzz {{raylibFlags}} {{flags}}&& ./build/puzz 

make_build_dir:
	mkdir -p build
