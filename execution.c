//Builtin command used in a pipeline actually does not change anything in the main process. 
//If you want to observe the effect of a builtin command, use it separately.
//The shell should regain control if a foreground process stops

#include "execution.h"

//utilities--------------------------------------------------------------------------------------------------------

int call_builtin(PIPE_LINE *cmd_seq, list *process_list, int i, int cases)
{
	int cd_st, env_st;

	if(cases == 0)
	{
		if(strcmp(cmd_seq->arglists[0][0],"cd") == 0){
		    if(cmd_seq->arglists[0][1] == NULL) cmd_seq->arglists[0][1] = strdup(getenv("HOME")); //no destination meand home
		    if(cmd_seq->arglists[0][2] != NULL) {fprintf(stderr,"error : cd : excess arguments\n"); return -1;}
		    cd_st = cd_wrapper(cmd_seq->arglists[0][1],cmd_seq->in_fname,cmd_seq->out_fname);
		    if(cd_st < 0) {fprintf(stderr,"error : cd\n"); return -1;}
		    return 0;
		}
		if(strcmp(cmd_seq->arglists[0][0],"jobs") == 0){
                    if(cmd_seq->arglists[0][1] == NULL){
			env_st = jobs_wrapper("0",process_list,cmd_seq->in_fname,cmd_seq->out_fname);
			if(env_st < 0) {fprintf(stderr,"error : jobs\n"); return -1;}
		    }
		    else if( strcmp(cmd_seq->arglists[0][1],"--help") == 0 ) 
			   jobs_wrapper(cmd_seq->arglists[0][1],process_list,cmd_seq->in_fname,cmd_seq->out_fname);
                    
		    else {fprintf(stderr,"error : wrong argument, check jobs --help\n"); return -1;}
                    return 0;
                }
		if(strcmp(cmd_seq->arglists[0][0],"unsetenv") == 0){
		    if(cmd_seq->arglists[0][1] == NULL) {fprintf(stderr,"error : unsetenv : no environment variable given\n"); return -1;}
		    if(cmd_seq->arglists[0][2] != NULL) {fprintf(stderr,"error : unsetenv : excess arguments\n"); return -1;}
		    env_st = unsetenv_wrapper(cmd_seq->arglists[0][1],cmd_seq->in_fname,cmd_seq->out_fname);
		    if(env_st < 0) {fprintf(stderr,"error : unsetenv\n"); return -1;}
		    return 0;
		}

		if(strcmp(cmd_seq->arglists[0][0],"fg") == 0){
			if(strcmp(cmd_seq->in_fname,"stdin") != 0){
				if(my_file_dup(cmd_seq->in_fname,0,0) < 0) return(-1);
			}
			if(strcmp(cmd_seq->out_fname,"stdout") != 0){
				if(my_file_dup(cmd_seq->out_fname,1,1) < 0) return(-1);
			}
		    if(cmd_seq->arglists[0][1] == NULL) {fprintf(stderr,"error : fg : no argument. Type fg --help\n"); return -1;}
		    if(cmd_seq->arglists[0][2] != NULL) {fprintf(stderr,"error : fg : excess arguments\n"); return -1;}
		    env_st = builtin_fg1(cmd_seq->arglists[0][1], process_list);
		    if(env_st < 0) {fprintf(stderr,"error : fg\n"); return -1;}
		    return 0;
		}   //not yet fullproof. check into it.
		
		if(strcmp(cmd_seq->arglists[0][0],"bg") == 0){
			if(strcmp(cmd_seq->in_fname,"stdin") != 0){
				if(my_file_dup(cmd_seq->in_fname,0,0) < 0) return(-1);
			}
			if(strcmp(cmd_seq->out_fname,"stdout") != 0){
				if(my_file_dup(cmd_seq->out_fname,1,1) < 0) return(-1);
			}
		    if(cmd_seq->arglists[0][1] == NULL) {fprintf(stderr,"error : bg : no argument. Type bg --help\n"); return -1;}
		    if(cmd_seq->arglists[0][2] != NULL) {fprintf(stderr,"error : bg : excess arguments\n"); return -1;}
		    env_st = builtin_bg(cmd_seq->arglists[0][1], process_list);
		    if(env_st < 0) {fprintf(stderr,"error : bg\n"); return -1;}
		    return 0;
		}

		if(strcmp(cmd_seq->arglists[0][0],"setenv") == 0){
		    if(setenv_wrapper((const char**)cmd_seq->arglists[0],cmd_seq->in_fname,cmd_seq->out_fname) == -1) return -1;
		    return 0;
		}
	}

	else
	{
	    if(strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],"cd") == 0){
		if(cmd_seq->arglists[cmd_seq->num_cmds-i][1] == NULL) cmd_seq->arglists[cmd_seq->num_cmds-i][1] = strdup(getenv("HOME"));  //no destination means home
		if(cmd_seq->arglists[cmd_seq->num_cmds-i][2] != NULL) {fprintf(stderr,"error : cd : excess arguments\n"); return(-1);}
		cd_st = cd_wrapper1(cmd_seq->arglists[cmd_seq->num_cmds-i][1]);
		if(cd_st < 0) {fprintf(stderr,"error : cd\n"); return(-1);}
		return(0);
	    }
	    if(strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],"jobs") == 0){
		    if(cmd_seq->arglists[cmd_seq->num_cmds-i][1] == NULL){
			env_st = builtin_jobs1("0",process_list);
			if(env_st < 0) {fprintf(stderr,"error : jobs\n"); return(-1);}
		    }
		    else if( strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][1],"--help") == 0 )
			   builtin_jobs1(cmd_seq->arglists[cmd_seq->num_cmds-i][1],process_list);

		    else {fprintf(stderr,"error : wrong argument, check jobs --help\n"); return(-1);}
		    return(0);
	    }

	    if(strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],"unsetenv") == 0){
		if(cmd_seq->arglists[cmd_seq->num_cmds-i][1] == NULL) {fprintf(stderr,"error : unsetenv : no environment variable given\n"); return(-1);}
		if(cmd_seq->arglists[cmd_seq->num_cmds-i][2] != NULL) {fprintf(stderr,"error : unsetenv : excess arguments\n"); return(-1);}
		env_st = builtin_unsetenv1(cmd_seq->arglists[cmd_seq->num_cmds-i][1]);
		if(env_st < 0) {fprintf(stderr,"error : unsetenv\n"); return(-1);}
		return(0);
	    }
	    if(strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],"fg") == 0){
		fprintf(stderr,"error : fg : called from background process\n");
		return(0);
	    }

	    if(strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],"bg") == 0){
		fprintf(stderr,"error : bg : called from background process\n");
		return(0);
	    }
	    if(strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],"setenv") == 0){
		if(setenv_wrapper((const char**)cmd_seq->arglists[cmd_seq->num_cmds-i],cmd_seq->in_fname,cmd_seq->out_fname) == -1) return(-1);
		return(0);
	    }
	}

	return 2;

}

//-----------------------------------------------------------------------------------------------------------------

int execution(PIPE_LINE *cmd_seq, list *process_list){
    static int i = 1;
    //fprintf(stderr,"value of i = %d\n",i);
    int dup2_st, exec_st, wait_st, pipe_st, status, wstatus, builtin_st, shell_gid;
    static int new_gid = 0;  //new_gid for background pipeline
    struct sigaction signal_act;  //for handling stop signal
    //fprintf(stderr,"value of i = %d\n",new_gid);
    //int temp_out = 0;
    if(i > cmd_seq->num_cmds) {i = 1; new_gid = 0; return 0;}

//there is a single command------------------------------------------------------------------------------------------------
    if(cmd_seq->num_cmds == 1){
	    if(cmd_seq->background)  //first check background option
	    {
		//if the command is a builtin directly run it no need to fork, you need to do input/output redirection as well
		builtin_st = call_builtin(cmd_seq, process_list, i, 0);
		if(builtin_st == 0 || builtin_st == -1) return builtin_st;
		//builtin--------------------------------------------------------------------------------------------------------
	    }
        status = fork();
        if(status == 0){
	    signal_default();  //default signaling in the children
	    //run in background--------------------------
	    if(cmd_seq->background == 0)
	    {
		setpgid(0,0);
	    }
	    //------------------------------------------
            if(strcmp(cmd_seq->in_fname,"stdin") != 0){
                if(my_file_dup(cmd_seq->in_fname,0,0) < 0) exit(-1);
            }
            if(strcmp(cmd_seq->out_fname,"stdout") != 0){
                if(my_file_dup(cmd_seq->out_fname,1,1) < 0) exit(-1);
            }
	    if(cmd_seq->background == 0)  //check if background is zero
	    {
		    //if command is a builtin run it here, do not execute, finally exit
		    builtin_st = call_builtin(cmd_seq, process_list, i, 1);
		    if(builtin_st == 0 || builtin_st == -1) exit(builtin_st);
		    //builtin--------------------------------------------------------------
	    }

            exec_st = execvp(cmd_seq->arglists[0][0],cmd_seq->arglists[0]);
            if(exec_st < 0) {fprintf(stderr,"error : exec : %s\n",cmd_seq->arglists[0][0]); exit(-1);}
        }
        else if(status > 0){

	    node *new = Makenode(cmd_seq->arglists[0][0],status,cmd_seq->job_id);  //can be needed if child stops
	    if(cmd_seq->background == 0)
	    {
		pushfront(process_list,new);
		/*wait_st = waitpid(status,&wstatus,WUNTRACED|WNOHANG);
		if(wait_st < 0) {fprintf(stderr,"error : wait\n"); return -1;}
		if( WIFSTOPPED(wstatus) ) fprintf(stderr,"%d stopped\n",status);
		else if( WIFEXITED(wstatus) ) {fprintf(stderr,"%d exited\n",status); new->status=1;}
		else if( WIFCONTINUED(wstatus) ) fprintf(stderr,"%d continued\n",status);*/
	    }

            if(cmd_seq->background){
                wait_st = waitpid(status,&wstatus,WUNTRACED);
		if(wait_st < 0) {fprintf(stderr,"error : wait\n"); return -1;}
		if( WIFSTOPPED(wstatus) )
		{
			fprintf(stderr,"%d stopped by signal\n",status);
			pushfront(process_list,new);  //add process to background process list
		}
            }

	    return 0; 
        }
	else{
		fprintf(stderr,"error : fork failed\n");
		return -1;
	}
    }
//there is a single command---------------------------------------------------------------------------------------

    else{
        int fdpipe[2];
        pipe_st = pipe(fdpipe);
        if(pipe_st < 0) {fprintf(stderr,"error : pipe\n"); return -1;}

        status = fork();
        if(status == 0){
	    signal_default();  //default signaling in the children 
	    //run in background----------------------------------------------
	    if(cmd_seq->background == 0)
	    {
		setpgid(0,new_gid);  //all processes in the pipeline should have same gid
	    }
	    //---------------------------------------------------------------
            if(i == cmd_seq->num_cmds){
                if(strcmp(cmd_seq->in_fname,"stdin") != 0){
                    if(my_file_dup(cmd_seq->in_fname,0,0) < 0) exit(-1);
                }
            }
            else{
                dup2_st = dup2(fdpipe[0],0);
                if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); exit(-1);}
            }

            close(fdpipe[0]);
            close(fdpipe[1]);

            if(i == 1){
                if(strcmp(cmd_seq->out_fname,"stdout") != 0){
                    if(my_file_dup(cmd_seq->out_fname,1,1) < 0) exit(-1);
                }
            }
            //if command is a builtin run it here, do not execute, finally exit
	    builtin_st = call_builtin(cmd_seq, process_list, i, 1);
	    if(builtin_st == 0 || builtin_st == -1) exit(builtin_st);
            //builtin--------------------------------------------------------------
            exec_st = execvp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],cmd_seq->arglists[cmd_seq->num_cmds-i]);
            if(exec_st < 0){
                fprintf(stderr,"error : exec : %s\n",cmd_seq->arglists[cmd_seq->num_cmds-i][0]);
                exit(-1);
            }
        }
        else if(status > 0){
	    
	    node *new = Makenode(cmd_seq->arglists[(cmd_seq->num_cmds) - i][0],status,cmd_seq->job_id);  //can be needed if process stops
	    if(cmd_seq->background == 0)
	    {
		pushfront(process_list,new);
		/*wait_st = waitpid(status,&wstatus,WUNTRACED|WNOHANG);
		if(wait_st < 0) {fprintf(stderr,"error : wait\n"); return -1;}
		if( WIFSTOPPED(wstatus) ) fprintf(stderr,"%d stopped\n",status);
		else if( WIFEXITED(wstatus) ) {fprintf(stderr,"%d exited\n",status); new->status = 1;}
		else if( WIFCONTINUED(wstatus) ) fprintf(stderr,"%d continued\n",status);*/
	    }
	    
	    if(i == 1) new_gid = status;  //all processes in the pipeline should have same gid

            dup2_st = dup2(fdpipe[1],1);
            if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
            close(fdpipe[0]);
            close(fdpipe[1]);
            i = i + 1;
            exec_st = execution(cmd_seq, process_list);
            if(exec_st < 0) return -1;
            //i--; 

            if(cmd_seq->background){
                wait_st = waitpid(status,&wstatus,WUNTRACED);  //if a child stops, shell should regain control
		if(wait_st < 0) {fprintf(stderr,"error : wait\n"); return -1;}
		if( WIFSTOPPED(wstatus) )
		{
			fprintf(stderr,"%d stopped by signal\n",status);
			pushfront(process_list,new);  //add process to background process list
		}
            }
 
            return 0;  
        }
	else{
		fprintf(stderr,"error : fork failed\n");
		return -1;
	}
    }
}

int exec_wrapper(PIPE_LINE *cmd_seq, list *process_list){
    int temp_in = dup(0);
    if(temp_in < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int temp_out = dup(1);
    if(temp_out < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int exec_st = execution(cmd_seq, process_list);  
    if(exec_st < 0) {fprintf(stderr,"error : execution\n"); return -1;}
    int dup2_st = dup2(temp_in,0);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    dup2_st = dup2(temp_out,1);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    close(temp_in);
    close(temp_out);
    
    return exec_st;
}


int find_path(int i, char ***arglists){
    int status,pipe_st,dup2_st,exec_st,wait_st,wstatus;
    int fdpipe[2];
    char **whichlist = (char **)malloc(3*sizeof(char *));
    whichlist[0] = "/usr/bin/which";
    whichlist[2] = NULL;
    
    whichlist[1] = arglists[i][0];
    pipe_st = pipe(fdpipe);
    if(pipe_st < 0) {fprintf(stderr,"error : pipe\n"); return -1;}
    status = fork();
    if(status == 0){
	signal_default();  //default signaling in children
        dup2_st = dup2(fdpipe[1],1);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); exit(-1);}
        close(fdpipe[0]);
        close(fdpipe[1]);
        exec_st = execv(whichlist[0],whichlist);
        if(exec_st < 0) {fprintf(stderr,"error : exec : which\n"); exit(-1);}
    }
    else if(status > 0){
        int temp_in = dup(0);
        if(temp_in < 0) {fprintf(stderr,"error : dup\n"); return -1;}
        dup2_st = dup2(fdpipe[0],0);
        close(fdpipe[0]);
        close(fdpipe[1]);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
        wait_st = waitpid(status,&wstatus,0);
        if(wait_st < 0) {fprintf(stderr,"error : wait\n"); return -1;}
        if(WEXITSTATUS(wstatus) == -1) return -1;
        scanf("%s",arglists[i][0]);
        dup2_st = dup2(temp_in,0);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
        close(temp_in);
    }
    else{
	    fprintf(stderr,"error : fork failed\n");
	    return -1;
    }
    
    return 0;
}

