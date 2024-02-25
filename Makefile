all: all_unix all_wasm

all_unix: build_std_unix_debug build_std_unix build_compiler_unix_debug build_compiler_unix

all_wasm: build_std_wasm build_compiler_wasm build_std_wasm_debug build_compiler_wasm_debug

build_std_unix_debug:
	gcc src/main.c -o ./bin/unix/debug/stapel -D DEBUG

build_std_unix:
	gcc src/main.c -o ./bin/unix/stapel

build_compiler_unix_debug:
	gcc compiler/main.c -o ./bin/unix/debug/stapelc -D DEBUG

build_compiler_unix:
	gcc compiler/main.c -o ./bin/unix/stapelc

build_std_wasm:
	emcc src/main.c -o ./bin/wasm/stapel.js -D WASM

build_compiler_wasm:
	emcc compiler/main.c -o ./bin/wasm/stapelc.js -D WASM

build_std_wasm_debug:
	emcc src/main.c -o ./bin/wasm/debug/stapel.js -D DEBUG -D WASM

build_compiler_wasm_debug:
	emcc compiler/main.c -o ./bin/wasm/debug/stapelc.js -D DEBUG -D WASM
