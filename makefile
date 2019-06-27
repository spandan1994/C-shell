build: shell

LFLAGS= -lreadline -lpam -lpam_misc
CFLAGS= -Wall -g

shell: lex.yy.c builtin.o execution.o process_list.o variable_list.o
	gcc -o shell lex.yy.c builtin.o execution.o process_list.o variable_list.o $(CFLAGS) $(LFLAGS)

lex.yy.c: myshell.l
	flex myshell.l

builtin.o: builtin.c
	gcc -c builtin.c

execution.o: execution.c
	gcc -c execution.c

process_list.o: process_list.c
	gcc -c process_list.c

variable_list.o: variable_list.c
	gcc -c variable_list.c

clean: 
	rm lex.yy.c
	rm shell
	rm *.o
