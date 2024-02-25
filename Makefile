all_unix: build_std_unix_debug build_std_unix build_compiler_debug build_compiler

build_std_unix_debug:
	gcc src/main.c -o ./bin/unix/debug/stapel  -D DEBUG

build_std_unix:
	gcc src/main.c -o ./bin/unix/stapel

build_compiler_debug:
	gcc compiler/main.c -o ./bin/unix/debug/stapelc  -D DEBUG

build_compiler:
	gcc compiler/main.c -o ./bin/unix/stapelc