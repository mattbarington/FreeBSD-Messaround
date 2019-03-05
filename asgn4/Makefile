
build:
	@cc hello.c -o hello `pkgconf fuse --cflags --libs`

run: build
	@mkdir newHelloFS
	@./hello newHelloFS
