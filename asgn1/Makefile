argshell: argshell.c lex.yy.c
	cc -o argshell argshell.c lex.yy.c -lfl

lex.yy.c: shell.l
	flex shell.l
