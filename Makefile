build_std_unix_debug:
	gcc src/main.c -o ./bin/stapel_unix  -D DEBUG

build_std_unix:
	gcc src/main.c -o ./bin/stapel_unix

build_test_files:
	nasm test/example.asm -o test/example.st

build_compiler_debug:
	gcc compiler/main.c -o ./bin/stapel_compiler_unix  -D DEBUG

build_compiler:
	gcc compiler/main.c -o ./bin/stapel_compiler_unix