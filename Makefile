all: all_unix all_wasm

all_unix: build_std_unix_debug build_std_unix build_compiler_unix

all_wasm: build_std_wasm build_compiler_wasm build_std_wasm_debug

build_std_unix_debug:
	gcc src/main.c -o ./bin/unix/debug/stapel -D DEBUG -D PROGRAM

build_std_unix:
	gcc src/main.c -o ./bin/unix/stapel -D PROGRAM

build_compiler_unix:
	gcc compiler/main.c -o ./bin/unix/stapelc -D PROGRAM

build_std_wasm:
	emcc src/main.c -o ./bin/wasm/stapel.js -D WASM -s EXPORTED_FUNCTIONS="['_setKbbuf','_handle_default_next_instruction']"

build_compiler_wasm:
	emcc compiler/main.c -o ./bin/wasm/stapelc.js -D WASM

build_std_wasm_debug:
	emcc src/main.c -o ./bin/wasm/debug/stapel.js -D DEBUG -D WASM -s EXPORTED_FUNCTIONS="['_setKbbuf','_handle_default_next_instruction']"

