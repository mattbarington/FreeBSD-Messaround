argshell: argshell.c lex.yy.c
	cc -o $@  argshell.c lex.yy.c -lfl

lex.yy.c: shell.l
	flex shell.l

clean:
	rm ./*~ ./argshell ./lex.yy.c

run:
	make
	./argshell
