
%{

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "execution.h"
#include "variable_list.h" 

#ifndef DEBUG
	#define DEBUG 
	FILE* logfile;
#endif

#define MAX_COMMANDS_NO 20
#define MAX_ARGS_NO 20						
#define MAX_PNAME_LEN PATH_MAX
#define MAX_UNAME_LEN LOGIN_NAME_MAX

enum {
	START = 0,
	COMMAND,
	PIPE,
	IREDIR,
	OREDIR,
	IFILE,
	OFILE,
	BACKGROUND_OPT,
	VAL,
	UNKNOWN
}; 

PIPE_LINE command_seq; //spandan
int exec_st = 0; //spandan (for checking exec status)
siginfo_t process_info; //spandan (for retrieving info of children)
list *process_list; //spandan (for storing info about background processes)
LIST *var_list; //spandan (for storing declared variables and their values)
char *var_name; //spandan (for dumping variable name that is to be searched)
NODE *Found; //spandan (for searching variable name)
struct termios term_in, term_out;  //for storing and restoring terminal during execution
struct sigaction signal_act;  //for signal handling

char* commands[MAX_COMMANDS_NO][MAX_ARGS_NO];
char* input_fname = NULL;
char* output_fname = NULL;
int background_opt = 0;

int state = START; //initialize state, command_no, arg_no
int command_no = 0;
int arg_no = 0;

//some flags------------------------------------------------------------
int f_valid = 0; //validity check
int f_cmd = 0;
int f_indir = 0;
int f_outdir = 0;
//flags----------------------------------------------------------------------

char user[MAX_UNAME_LEN];
char cwd[MAX_PNAME_LEN];

char prompt[MAX_PNAME_LEN + MAX_UNAME_LEN + 10];

static struct pam_conv conv = 
{
    misc_conv,
    NULL
};


%}

%x ASSIGN EXACT

%%

^[ \t]*[^:| \t\n"><&]+[ \t]*:	{	
					char *declare_text = strdup(yytext);
					int i;
					for(i = 0 ; declare_text[i] != ':' && declare_text[i] != ' ' ; i++);
					declare_text[i] = '\0';
					commands[command_no][arg_no] = strdup(declare_text);
					free(declare_text);
					arg_no++;
					f_valid = -1;
					BEGIN ASSIGN;
				}

<EXACT>[^:"><&]+	{
				if(state == VAL)
				{
					f_valid = 1;
					state = VAL;
					commands[command_no][arg_no] = strdup(yytext);
					arg_no++;
				}
				else if(state == COMMAND)
				{
					f_valid = 0;
					state = COMMAND;
					commands[command_no][arg_no] = strdup(yytext);
					arg_no++;
				}
				else
				{
					f_valid = -1;
					state = BACKGROUND_OPT;
				}
			}

<EXACT>["]	{
			if(state == VAL)
			{
				if(f_valid == -1) state = BACKGROUND_OPT;
				else state = VAL;
				BEGIN INITIAL;
			}
			else if(state == COMMAND)
			{
				if(f_valid == -1) state = BACKGROUND_OPT;
				else state = COMMAND;
				BEGIN INITIAL;
			}
			else
			{
				f_valid = -1;
				state = BACKGROUND_OPT;
				BEGIN INITIAL;
			}
		}	


<ASSIGN>[ \t\n]+	;  //ignore;

<ASSIGN>[^:| \t\n"><&]+	{
				if(state == START)
				{
					f_valid = 1;   //special case for variable assignment
					state = VAL;
					commands[command_no][arg_no] = strdup(yytext);
					arg_no++;
					BEGIN INITIAL;	
				}	
				else
				{
					f_valid = -1;
					state = BACKGROUND_OPT;
					BEGIN INITIAL;
				}
			}

<ASSIGN>["]	{
			if(state == START)
			{
				f_valid = -1;
				state = VAL;
				BEGIN EXACT;
			}
			else
			{
				f_valid = -1;
				state = BACKGROUND_OPT;
				BEGIN INITIAL;
			}
		}

<ASSIGN>[|:><&]+	{
				f_valid = -1;
				state = BACKGROUND_OPT;
				BEGIN INITIAL;
			}



[ \t\n]+	; 		//ignore

["]	{
		if(state == COMMAND)
		{
			f_valid = -1;
			state = COMMAND;
			BEGIN EXACT;
		}
		else if(state == VAL)
		{
			f_valid = -1;
			state = VAL;
			BEGIN EXACT;
		}
		else
		{
			f_valid = -1;
			state = BACKGROUND_OPT;
		}
	}

[^:| \t\n"><&]+	{
			if(state == START)
			{
				f_cmd = 1;
				/*search for the word in case it should be replaced*/
				var_name = strdup(yytext);
				Found = Search_by_var(var_list,var_name);
				int i;
				if(Found != NULL)
				{
					for(i = 1 ; (Found->variable[i]) != NULL ; i++)
					{
						commands[command_no][arg_no] = strdup( (Found->variable[i]) );
						arg_no++;
					}		
				}
				else
				{	
					commands[command_no][arg_no] = strdup(var_name);
					arg_no++;
				}
				free(var_name);
				//--------------------------------------------------
				if( strcmp(commands[0][0],"exit") ) f_valid = 0;
				else f_valid = -2;  //for exiting from the shell
				state = COMMAND;
			}
			else if(state == VAL)
			{
				commands[command_no][arg_no] = strdup(yytext);
				arg_no++;
				f_valid = 1;
				state = VAL;
			}
			else if(state == COMMAND)
			{
				/*search in case of replacement*/
				var_name = strdup(yytext);
				Found = Search_by_var(var_list,var_name);
				int i;
				if(Found != NULL && (Found->variable[0][0]) == '$')  //argument cannot be shell variable, only user defined
				{
					for(i = 1 ; (Found->variable[i]) != NULL ; i++)
					{
						commands[command_no][arg_no] = strdup( (Found->variable[i]) );
						arg_no++;
					}		
				}
				else
				{	
					commands[command_no][arg_no] = strdup(var_name);
					arg_no++;
				}
				free(var_name);
				//--------------------------------------------------
				f_valid = 0;
				state = COMMAND;
			}
			else if(state == PIPE)
			{
				/*search in case of replacement*/
				var_name = strdup(yytext);
				Found = Search_by_var(var_list,var_name);
				int i;
				if(Found != NULL)
				{
					for(i = 1 ; (Found->variable[i]) != NULL ; i++)
					{
						commands[command_no][arg_no] = strdup( (Found->variable[i]) );
						arg_no++;
					}		
				}
				else
				{	
					commands[command_no][arg_no] = strdup(var_name);
					arg_no++;
				}
				free(var_name);
				//--------------------------------------------------
				f_valid = 0;
				state = COMMAND;
			}
			else if(state == IREDIR)
			{
				input_fname = strdup(yytext);
				f_valid = 0;
				state = IFILE;
			}
			else if(state == OREDIR)
			{
				output_fname = strdup(yytext);
				f_valid = 0;
				state = OFILE;
			}
			else if(state == IFILE || state == OFILE)
			{
				if(f_cmd)
				{
					f_valid = -1;
					state = BACKGROUND_OPT;
				}
				else
				{
					f_cmd = 1;
					/*search for the word in case it should be replaced*/
					var_name = strdup(yytext);
					Found = Search_by_var(var_list,var_name);
					int i;
					if(Found != NULL)
					{
						for(i = 1 ; (Found->variable[i]) != NULL ; i++)
						{
							commands[command_no][arg_no] = strdup( (Found->variable[i]) );
							arg_no++;
						}		
					}
					else
					{	
						commands[command_no][arg_no] = strdup(var_name);
						arg_no++;
					}
					free(var_name);
					//--------------------------------------------------
					f_valid = 0;
					state = COMMAND;
				}	
			}
			else
			{
				f_valid = -1;
				state = BACKGROUND_OPT;
			}
		}

">" |
">>" 	{
		if(f_outdir)
		{
			f_valid = -1;
			state = BACKGROUND_OPT;
		}
		else
		{
			if(state == COMMAND || state == IFILE || state == START)
			{
				f_outdir = 1;
				f_valid = -1;
				state = OREDIR;
			}
			else
			{
				f_valid = -1;
				state = BACKGROUND_OPT;
			}
		}
	}

"<" |
"<<"	{
		if(f_indir)
		{
			f_valid = -1;
			state = BACKGROUND_OPT;
		}
		else
		{
			if(state == COMMAND || state == OFILE || state == START)
			{
				f_indir = 1;
				f_valid = -1;
				state = IREDIR;
			}
			else
			{
				f_valid = -1;
				state = BACKGROUND_OPT;
			}
		}
	}

"|"	{
		if(state == COMMAND)
		{
			f_valid = -1;
			state = PIPE;
			command_no++;
			arg_no = 0;
		}
		else
		{
			f_valid = -1;
			state = BACKGROUND_OPT;
		}
	}

"&"	{
		if(state == COMMAND || state == IFILE || state == OFILE)
		{
			background_opt = 1;
			f_valid = 0;
			state = BACKGROUND_OPT;
		}
		else
		{
			f_valid = -1;
			state = BACKGROUND_OPT;
		}
	}

":"	{
		f_valid = -1;
		state = BACKGROUND_OPT;
	}


%%

int yywrap()
{
	return 1;

}

void print_command_storage(void)
{
	#ifdef DEFINE
	for(int i = 0; commands[i][0] !=  NULL; i++)
	{
		fprintf(logfile, "command: ");
		for(int j = 0; commands[i][j] !=  NULL; j++)
		{
			printf(logfile, "%s ", commands[i][j]);
		}
		printf(logile, "\n");
	}
	printf(logfile, "input: %s output: %s backopt %d\n", input_fname, output_fname, background_opt);
	fflush(logfile);
	#endif

	
	return;

}

void prepare_exec_env(void)
{
	int i,j,m;
	command_seq.num_cmds = command_no + 1;
	command_seq.arglists = (char ***)malloc(command_seq.num_cmds * sizeof(char **));
	
	for(i=0 ; i<command_seq.num_cmds ; i++)
	{
        	for(m=0 ; commands[i][m] != NULL ; m++);
		m = m + 1;
        	command_seq.arglists[i] = (char **)malloc((m+1) * sizeof(char *));
        	for(j=0 ; j<m ; j++)
		{
            		command_seq.arglists[i][j] = commands[i][j];
        	}
        	command_seq.arglists[i][j] = NULL;
    	}
	if(input_fname)
		strcpy(command_seq.in_fname,input_fname);
	else
		strcpy(command_seq.in_fname,"stdin");

	if(output_fname)
		strcpy(command_seq.out_fname,output_fname);
	else
		strcpy(command_seq.out_fname,"stdout");
	
	command_seq.background = 1 - background_opt;

	return;
}

void print_exec_env(void)
{
	#ifdef DEBUG
	for(int i = 0 ; i < command_seq.num_cmds ; i++)
	{
		fprintf(logfile, "command: ");
		for(int j = 0 ; command_seq.arglists[i][j] !=  NULL ; j++)
		{
			fprintf(logfile, "%s ", command_seq.arglists[i][j]);
		}
		fprintf(logfile, "\n");
	}
	fprintf(logfile, "input %s output %s backopt %d\n", command_seq.in_fname, command_seq.out_fname, command_seq.background);
	fflush(logfile);
	#endif 

	return;
}

void reset_command_storage(void)
{
	for(int i = 0; i < MAX_COMMANDS_NO ; i++)
	{
		for(int j = 0; j < MAX_ARGS_NO; j++)
		{
			commands[i][j] = NULL;
		}
	}
	background_opt = 0;
	input_fname = output_fname = NULL;
	
	state = START;
	command_no = 0;
	arg_no = 0;
//reset flags---------------------------------------------
	f_valid = 0;
	f_cmd = 0;
	f_indir = 0;
	f_outdir = 0;
//--------------------------------------------------------
	
	BEGIN INITIAL;  //reset lex start state to INITIAL	
	return;
}

int authenticate_user(const char* uname)
{
    int ret;
    const char* user;
    
    user = uname;
    
    pam_handle_t *pamh = NULL;
    ret = pam_start("auth", user, &conv, &pamh);
    
    if (ret == PAM_SUCCESS)
        ret = pam_authenticate(pamh, 0);   

    if (ret == PAM_SUCCESS)
        ret = pam_acct_mgmt(pamh, 0);      


    if (ret == PAM_SUCCESS) 
    {
        fprintf(logfile, "user authenticated\n");
    } 
    else 
    {
        fprintf(logfile , "user not authenticated\n");
	exit(1);
    }

    if (pam_end(pamh, ret) != PAM_SUCCESS) 
    {     
        pamh = NULL;
        fprintf(logfile, "auth: failed to release authenticator\n");
        exit(1);
    }

    return ( ret == PAM_SUCCESS ? 0:1 );
}

void free_exec_env(void)
{
	int i,j;
	for(i = 0 ; i < command_seq.num_cmds ; i++)
	{
		for(j = 0 ; command_seq.arglists[i][j] != NULL ; j++)
		{
			free(command_seq.arglists[i][j]);
		}
		free(command_seq.arglists[i][j]);
		free(command_seq.arglists[i]);
	}
	free(command_seq.arglists);
	command_seq.num_cmds = 0;
	command_seq.background = 1;

	return;
}

void sigint_handler(int signo)
{
	fprintf(stderr,"exit by typing 'exit'\n");
}

/*int custom_exit(const char *text, int state)
{
	exit(0);
	fprintf("stderr : exit\n");	
	return 0;
}*/

void setup_rc(void)
{
	FILE *fp = fopen("shellrc","r");
	if(feof(fp))
	{	
		printf("Setting up rc file for once\n");	
		fclose(fp);
		fp = fopen("shellrc","w");
		fprintf(fp,"$include /etc/inputrc\n\n");
		fprintf(fp,"set editing-mode emacs\n\n");
		fprintf(fp,"~: \"%s\"\n",getenv("HOME"));
	}
	else fclose(fp);
}


void set_default_variables(void)
{
	commands[0][0] = "ls"; commands[0][1] = "ls"; commands[0][2] = "--color=auto";
	Found = MakeNODE(commands[0]);
	pushFRONT(var_list, Found);
	reset_command_storage();
	
	commands[0][0] = "ll"; commands[0][1] = "ls"; commands[0][2] = "--color=auto"; commands[0][3] = "-al";
	Found = MakeNODE(commands[0]);
	pushFRONT(var_list, Found);
	reset_command_storage();

	commands[0][0] = "grep"; commands[0][1] = "grep"; commands[0][2] = "--color=auto";
	Found = MakeNODE(commands[0]);
	pushFRONT(var_list, Found);
	reset_command_storage();
}


int main(int argc, char** argv) 
{
	int ret;
	char *eval_string = (char *)malloc(100 * sizeof(char));  //for variable substitution	
	#ifdef DEBUG
	if((logfile = fopen("shell.log", "w") ) == NULL)
	{
		exit(1);	
	}
	#endif

	printf("username: ");
	scanf("%20s", user);
	authenticate_user(user);
    
	//rl_bind_key('\t', rl_insert);
	//rl_bind_keyseq("exit",custom_exit);
	char* buf;
	//char* input;

	system("clear");

//path of inputrc file------------------------------------------------
	setup_rc();
	setenv("INPUTRC","./shellrc",1);
//inputrc--------------------------------------------------------------

//signal_ignore------------------------------------------------------------
	signal_act.sa_handler = sigint_handler;
	sigaction(SIGINT, &signal_act, NULL);
	signal_act.sa_handler = SIG_IGN;
	sigaction(SIGTSTP, &signal_act, NULL);
	sigaction(SIGTTOU, &signal_act, NULL);
	sigaction(SIGQUIT, &signal_act, NULL);
	sigaction(SIGHUP, &signal_act, NULL);
//signal_ignore------------------------------------------------------------
		
	process_list = Createlist(); //for storing background process info
	var_list = CreateLIST(); //for storing declared variable and their values	
	set_default_variables(); //set some default variables

	for(int job = 1 ; ;)
	{
		getcwd(cwd,  sizeof(cwd)); 
		sprintf(prompt, ANSI_COLOR_GREEN "%s @ %s >>> " ANSI_COLOR_RESET , user, cwd);
//check for background processes--------------------------------------------------------------------------------------------
		node *temp = process_list->head;
		while(temp != NULL)
		{
			process_info.si_pid = 0;
			waitid(P_PID,temp->p_pid,&process_info,WEXITED|WSTOPPED|WCONTINUED|WNOHANG);  
			if(process_info.si_pid > 0)
			{
				if(process_info.si_code == CLD_STOPPED) printf("%d stopped\n",process_info.si_pid);
				else if(process_info.si_code == CLD_EXITED)
				{
					printf("%d exited\n",process_info.si_pid);
					temp->status = 1;
				}
				else if(process_info.si_code == CLD_KILLED)
				{
					printf("%d killed\n",process_info.si_pid);
					temp->status = 1;
				}
				else if(process_info.si_code == CLD_DUMPED) 
				{
					printf("%d core dumped\n",process_info.si_pid);
					temp->status = 1;
				}
				else if(process_info.si_code == CLD_TRAPPED) printf("%d trapped\n",process_info.si_pid);
				else if(process_info.si_code == CLD_CONTINUED) printf("%d continued\n",process_info.si_pid);
				else printf("abnormal behaviour of child %d\n",exec_st);
			}
			temp = temp->next;
		}
		Update(process_list);
//background processes---------------------------------------------------------------------------------------------------------------

		if((buf = readline(prompt)) != NULL )
		{
			if (strlen(buf) > 0) 
			{
  				add_history(buf);
				reset_command_storage();
				yy_scan_string(buf);
				do{ ret = yylex(); puts(yytext); } while(ret != 0);
				yy_delete_buffer(YY_CURRENT_BUFFER);
				free(buf);
				print_command_storage();

//check for syntax error--------------------------------------------------------------------------------------------------------------
				if(f_valid == -2) break;
				else if(f_valid == -1) printf("Invalid syntax\n");
				else if(f_valid == 1)
				{
					eval_string[0] = '$'; eval_string[1] = '\0';
					strcat(eval_string,commands[0][0]);
					free(commands[0][0]);
					commands[0][0] = strdup(eval_string);
					ret = Search_and_replace(var_list, commands[0]);
					if(ret == 0)
					{
						NODE *NEWnode = MakeNODE(commands[0]);
						pushFRONT(var_list,NEWnode);
					}
				} 
				else 
				{
					if(f_cmd == 0) printf("No command to execute\n");
					
					else
					{
						prepare_exec_env();
						command_seq.job_id = job;  //job id for this job
						print_exec_env();


						/*for(int i=0 ; i<command_seq.num_cmds ; i++)
						{
							int find_st = find_path(i,command_seq.arglists);
							if(find_st < 0) fprintf(logfile,"error : find_path\n");
						}*/
						
						node *current_head = process_list->head;   //marks the current head position

						tcgetattr(STDIN_FILENO,&term_in);
						tcgetattr(STDOUT_FILENO,&term_out);
						exec_st = exec_wrapper(&command_seq, process_list);
						if(exec_st < 0) fprintf(logfile,"error : exec_wrapper\n");
						tcsetattr(STDIN_FILENO,TCSADRAIN,&term_in);
						tcsetattr(STDOUT_FILENO,TCSADRAIN,&term_out);
					
						if(current_head != process_list->head) job = job+1;  //change job_id if process list has changed
						free_exec_env();
					}
				}
//syntax error------------------------------------------------------------------------------------------------------------------------

			
			}
			
			else free(buf);			
		
		}
	
	}

	free_list(process_list);
	Free_LIST(var_list);
  	return 0;
}

