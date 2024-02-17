build_std_unix:
	gcc src/main.c -o ./bin/stapel_unix  -D DEBUG

build_test_files:
	nasm test/example.asm -o test/example.st