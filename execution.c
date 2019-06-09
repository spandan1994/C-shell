//Builtin command used in a pipeline actually does not change anything in the main process. 
//If you want to observe the effect of a builtin command, use it separately.

#include "execution.h"

//utilities--------------------------------------------------------------------------------------------------------

void signal_ignore(void)
{
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);
	signal (SIGTSTP, SIG_IGN);
	signal (SIGTTIN, SIG_IGN);
	signal (SIGTTOU, SIG_IGN);
	signal (SIGCHLD, SIG_IGN);
}

void signal_default(void)
{
	signal (SIGINT, SIG_DFL);
        signal (SIGQUIT, SIG_DFL);
        signal (SIGTSTP, SIG_DFL);
        signal (SIGTTIN, SIG_DFL);
        signal (SIGTTOU, SIG_DFL);
        signal (SIGCHLD, SIG_DFL);
}


int my_file_dup(char *fname, int mode, int fd){ //opens a file with name fname and given mode and dup2 it at fd
    int new_fd, dup2_st;
    if(mode == 0){
        new_fd = open(fname,O_RDONLY);
        if(new_fd < 0) {fprintf(stderr,"error : open\n"); return -1;}
    }
    else if(mode == 1){
        new_fd = open(fname,O_CREAT|O_WRONLY|O_TRUNC,00600);
        if(new_fd < 0) {fprintf(stderr,"error : open\n"); return -1;}
    }
    else{
        new_fd = open(fname,O_CREAT|O_APPEND,00600);
        if(new_fd < 0) {fprintf(stderr,"error : open\n"); return -1;}
    }
    dup2_st = dup2(new_fd,fd);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    close(new_fd);

    return 0;
}

//-----------------------------------------------------------------------------------------------------------------



int execution(PIPE_LINE *cmd_seq, list *process_list){
    static int i = 1;
    //fprintf(stderr,"value of i = %d\n",i);
    int dup2_st, exec_st, wait_st, pipe_st, status, wstatus, cd_st, env_st;
    static int new_gid = 0;  //new_gid for background pipeline
    //fprintf(stderr,"value of i = %d\n",new_gid);
    //int temp_out = 0;
    if(i > cmd_seq->num_cmds) {i = 1; new_gid = 0; return 0;}

//there is a single command------------------------------------------------------------------------------------------------
    if(cmd_seq->num_cmds == 1){
	    if(cmd_seq->background)  //first check background option
	    {
		//if the command is a builtin directly run it no need to fork, you need to do input/output redirection as well
		if(strcmp(cmd_seq->arglists[0][0],"cd") == 0){
		    if(cmd_seq->arglists[0][1] == NULL) {fprintf(stderr,"error : cd : no destination given\n"); return -1;}
		    if(cmd_seq->arglists[0][2] != NULL) {fprintf(stderr,"error : cd : excess arguments\n"); return -1;}
		    cd_st = cd_wrapper(cmd_seq->arglists[0][1],cmd_seq->in_fname,cmd_seq->out_fname);
		    if(cd_st < 0) {fprintf(stderr,"error : cd\n"); return -1;}
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
		    if(cmd_seq->arglists[0][1] == NULL) {fprintf(stderr,"error : fg : no argument. Type fg --help\n"); return -1;}
		    if(cmd_seq->arglists[0][2] != NULL) {fprintf(stderr,"error : fg : excess arguments\n"); return -1;}
		    env_st = fg_wrapper(cmd_seq->arglists[0][1], process_list);
		    if(env_st < 0) {fprintf(stderr,"error : fg\n"); return -1;}
		    return 0;
		}   //not yet fullproof. check into it.

		if(strcmp(cmd_seq->arglists[0][0],"setenv") == 0){
		    if(setenv_wrapper((const char**)cmd_seq->arglists[0],cmd_seq->in_fname,cmd_seq->out_fname) == -1) return -1;
		    return 0;
		}
		//builtin--------------------------------------------------------------------------------------------------------
	   }
        status = fork();
        if(status == 0){
	    //signal_default();  //default signaling in the children
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
		    if(strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],"cd") == 0){
			if(cmd_seq->arglists[0][1] == NULL) {fprintf(stderr,"error : cd : no destination given\n"); exit(-1);}
			if(cmd_seq->arglists[0][2] != NULL) {fprintf(stderr,"error : cd : excess arguments\n"); exit(-1);}
			cd_st = cd_wrapper1(cmd_seq->arglists[i][1]);
			if(cd_st < 0) {fprintf(stderr,"error : cd\n"); exit(-1);}
			exit(0);
		    }
		    if(strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],"unsetenv") == 0){
			if(cmd_seq->arglists[0][1] == NULL) {fprintf(stderr,"error : unsetenv : no environment variable given\n"); exit(-1);}
			if(cmd_seq->arglists[0][2] != NULL) {fprintf(stderr,"error : unsetenv : excess arguments\n"); exit(-1);}
			env_st = builtin_unsetenv1(cmd_seq->arglists[i][1]);
			if(env_st < 0) {fprintf(stderr,"error : unsetenv\n"); exit(-1);}
			exit(0);
		    }
		    if(strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],"fg") == 0){
			fprintf(stderr,"error : fg : called from background process\n");
			exit(0);
		    }
		    if(strcmp(cmd_seq->arglists[0][0],"setenv") == 0){
			if(setenv_wrapper((const char**)cmd_seq->arglists[0],cmd_seq->in_fname,cmd_seq->out_fname) == -1) exit(-1);
			exit(0);
		    }
		    //builtin--------------------------------------------------------------
	    }

            exec_st = execv(cmd_seq->arglists[0][0],cmd_seq->arglists[0]);
            if(exec_st < 0) {fprintf(stderr,"error : exec : %s\n",cmd_seq->arglists[0][0]); exit(-1);}
        }
        else{
	    if(cmd_seq->background == 0)
	    {
		node *new = Makenode(cmd_seq->arglists[0][0],status);
		pushfront(process_list,new);
	    }

            if(cmd_seq->background){
                wait_st = waitpid(status,&wstatus,0);
		if(wait_st < 0) {fprintf(stderr,"error : wait\n"); return -1;}
            }
            /*else{
                wait_st = waitpid(status,&wstatus,WNOHANG);
            }
            if(wait_st < 0) {fprintf(stderr,"error : wait\n"); return -1;}
            if(WEXITSTATUS(wstatus) == -1) return -1;*/
	    return 0; 
        }
    }
//there is a single command---------------------------------------------------------------------------------------

    else{
        int fdpipe[2];
        pipe_st = pipe(fdpipe);
        if(pipe_st < 0) {fprintf(stderr,"error : pipe\n"); return -1;}

        status = fork();
        if(status == 0){
	    //signal_default();  //default signaling in the children
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
            if(strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],"cd") == 0){
                if(cmd_seq->arglists[0][1] == NULL) {fprintf(stderr,"error : cd : no destination given\n"); exit(-1);}
                if(cmd_seq->arglists[0][2] != NULL) {fprintf(stderr,"error : cd : excess arguments\n"); exit(-1);}
                cd_st = cd_wrapper1(cmd_seq->arglists[i][1]);
                if(cd_st < 0) {fprintf(stderr,"error : cd\n"); exit(-1);}
                exit(0);
            }
            if(strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],"unsetenv") == 0){
                if(cmd_seq->arglists[0][1] == NULL) {fprintf(stderr,"error : unsetenv : no environment variable given\n"); exit(-1);}
                if(cmd_seq->arglists[0][2] != NULL) {fprintf(stderr,"error : unsetenv : excess arguments\n"); exit(-1);}
                env_st = builtin_unsetenv1(cmd_seq->arglists[i][1]);
                if(env_st < 0) {fprintf(stderr,"error : unsetenv\n"); exit(-1);}
                exit(0);
            }
            if(strcmp(cmd_seq->arglists[cmd_seq->num_cmds-i][0],"fg") == 0){
		fprintf(stderr,"error : fg : not called from main process\n");
		exit(0);
            }
            if(strcmp(cmd_seq->arglists[0][0],"setenv") == 0){
                if(setenv_wrapper((const char**)cmd_seq->arglists[0],cmd_seq->in_fname,cmd_seq->out_fname) == -1) exit(-1);
                exit(0);
            }
            //builtin--------------------------------------------------------------
            exec_st = execv(cmd_seq->arglists[cmd_seq->num_cmds-i][0],cmd_seq->arglists[cmd_seq->num_cmds-i]);
            if(exec_st < 0){
                fprintf(stderr,"error : exec : %s\n",cmd_seq->arglists[cmd_seq->num_cmds-i][0]);
                exit(-1);
            }
        }
        else{
	    if(cmd_seq->background == 0)
	    {
		node *new = Makenode(cmd_seq->arglists[(cmd_seq->num_cmds) - i][0],status);
		pushfront(process_list,new);
	    }
            /*temp_out = dup(1);
            if(temp_out < 0) {fprintf(stderr,"error : dup\n"); return -1;}*/
	    if(i == 1) new_gid = status;  //all processes in the pipeline should have same gid

            dup2_st = dup2(fdpipe[1],1);
            if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
            close(fdpipe[0]);
            close(fdpipe[1]);
            i++;
            exec_st = execution(cmd_seq, process_list);
            if(exec_st < 0) return -1;
            //i--;

            /*dup2_st = dup2(temp_out,1);
            if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
            close(temp_out);*/

            if(cmd_seq->background){
                wait_st = waitpid(status,&wstatus,0);
		if(wait_st < 0) {fprintf(stderr,"error : wait\n"); return -1;}
            }
            /*else{
                wait_st = waitpid(-1,&wstatus,WNOHANG);
            }
            if(wait_st < 0) {fprintf(stderr,"error : wait\n"); return -1;}
            if(WEXITSTATUS(wstatus) == -1) return -1;*/
            return 0;  
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
        dup2_st = dup2(fdpipe[1],1);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); exit(-1);}
        close(fdpipe[0]);
        close(fdpipe[1]);
        exec_st = execv(whichlist[0],whichlist);
        if(exec_st < 0) {fprintf(stderr,"error : exec : which\n"); exit(-1);}
    }
    else{
        int temp_in = dup(0);
        if(temp_in < 0) {fprintf(stderr,"error : dup\n"); return -1;}
        dup2_st = dup2(fdpipe[0],0);
        close(fdpipe[0]);
        close(fdpipe[1]);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
        wait_st = waitpid(-1,&wstatus,0);
        if(wait_st < 0) {fprintf(stderr,"error : wait\n"); return -1;}
        if(WEXITSTATUS(wstatus) == -1) return -1;
        scanf("%s",arglists[i][0]);
        dup2_st = dup2(temp_in,0);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
        close(temp_in);
    }
    
    return 0;
}

