build: shell

LFLAGS= -lreadline -lpam -lpam_misc
CFLAGS= -Wall -g

shell: lex.yy.c builtin.o execution.o
	gcc -o shell lex.yy.c builtin.o execution.o $(CFLAGS) $(LFLAGS)

lex.yy.c: myshell.l
	flex myshell.l

builtin.o: builtin.c
	gcc -c builtin.c

execution.o: execution.c
	gcc -c execution.c

clean: 
	rm lex.yy.c
	rm shell
	rm *.o
