
%{

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
	ARG,
	PIPE,
	IREDIR,
	OREDIR,
	IFILE,
	OFILE,
	BACKGROUND_OPT,
	UNKNOWN
}; 

PIPE_LINE command_seq; //spandan

char* commands[MAX_COMMANDS_NO][MAX_ARGS_NO];
char* input_fname = NULL;
char* output_fname = NULL;
int background_opt = 0;

int state;
int command_no;
int arg_no;

char user[MAX_UNAME_LEN];
char cwd[MAX_PNAME_LEN];

char prompt[MAX_PNAME_LEN + MAX_UNAME_LEN + 10];

static struct pam_conv conv = 
{
    misc_conv,
    NULL
};


%}


%%

[ \t\n]+ 			; 		//ignore

[^| \t\n><&]+		{
				switch(state)
				{
					case COMMAND:	
							arg_no = 0;
							commands[command_no][arg_no] = strdup(yytext);
							arg_no++;
							state = ARG;
							break;
					case ARG:	
							commands[command_no][arg_no] = strdup(yytext);
							arg_no++;
							state = ARG;
							break;
					case IREDIR:	
							input_fname = strdup(yytext);
							state = BACKGROUND_OPT;
							break;
					case OREDIR:	
							output_fname = strdup(yytext);
							state = BACKGROUND_OPT;
							break;
					default:
							state= UNKNOWN;
							break;
				}
			}

">" |
">>" 			{
				state = OREDIR; 
			}

"<" |
"<<"			{
				state = IREDIR; 
			}

"|"			{
				command_no++;
				state = COMMAND;
			}

"&"			{
				if(state = BACKGROUND_OPT)
					{
						background_opt = 1;
					}
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
	
	state = COMMAND;
	command_no = 0;
	arg_no = 0;
	
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
	for(i = 0 ; i < command_seq.num_cmds ; i++){
		for(j = 0 ; command_seq.arglists[i][j] != NULL ; j++){
			free(command_seq.arglists[i][j]);
		}
		free(command_seq.arglists[i][j]);

		free(command_seq.arglists[i]);
	}
	free(command_seq.arglists);
}



int main(int argc, char** argv) 
{
	int ret;
	
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
	char* buf;
	char* input;

	system("clear");

	for(;;)
	{
		getcwd(cwd,  sizeof(cwd)); 
		sprintf(prompt, "%s @ %s >>> ", user, cwd);
		if(waitpid(-1,NULL,WNOHANG) == 0) printf("background processes running\n");
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
				prepare_exec_env();
				print_exec_env();


				for(int i=0 ; i<command_seq.num_cmds ; i++)
				{
					int find_st = find_path(i,command_seq.arglists);
					if(find_st < 0) fprintf(logfile,"error : find_path\n");
				}

				int exec_st = exec_wrapper(&command_seq);
        			if(exec_st < 0) fprintf(logfile,"error : exec_wrapper\n");
			
				//freeing space
				free_exec_env();
			}
			
			else free(buf);			
		
		}
	
	}

  	return 0;
}

