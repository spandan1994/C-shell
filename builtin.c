//The shell should regain control if a foreground process stops

#include "builtin.h"

//utilities---------------------------------------------------------------------------
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
        new_fd = open(fname,O_CREAT|O_APPEND|O_WRONLY,00600);
        if(new_fd < 0) {fprintf(stderr,"error : open\n"); return -1;}
    }
    dup2_st = dup2(new_fd,fd);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    close(new_fd);

    return 0;
}

void signal_ignore(void)
{
        struct sigaction signal_act;
	signal_act.sa_handler = SIG_IGN;
	sigaction (SIGINT, &signal_act, NULL);
        sigaction (SIGQUIT, &signal_act, NULL);
        sigaction (SIGTTIN, &signal_act, NULL);
        sigaction (SIGTTOU, &signal_act, NULL);
        sigaction (SIGCHLD, &signal_act, NULL);
	sigaction (SIGHUP, &signal_act, NULL);
        sigaction (SIGTSTP, &signal_act, NULL);
}

void signal_default(void)
{
        struct sigaction signal_act;
	signal_act.sa_handler = SIG_DFL;
	sigaction (SIGINT, &signal_act, NULL);
        sigaction (SIGQUIT, &signal_act, NULL);
        sigaction (SIGTTIN, &signal_act, NULL);
        sigaction (SIGTTOU, &signal_act, NULL);
        sigaction (SIGCHLD, &signal_act, NULL);
	sigaction (SIGHUP, &signal_act, NULL);
        sigaction (SIGTSTP, &signal_act, NULL);
}
//utilities_end-----------------------------------------------------------------------

//not yet fullproof, needs checking----------------------------------------------------------------

int search_env(char *command){
    int status,pipe_st,dup2_st,exec_st,wait_st,wstatus;
    int fdpipe[2];
    char **whichlist = (char **)malloc(3*sizeof(char *));
    whichlist[0] = "/usr/bin/which";
    whichlist[2] = NULL;
    
    whichlist[1] = command;
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
        scanf("%s",command);
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


int builtin_bg(char *path_name, list *process_list)
{
    pid_t shell_GID = getpgid(0);
    if(strcmp(path_name,"--help") == 0){
        printf("Synopsis : bg pname\n");
        printf("           bg --help\n\n");
        printf("Description :\nbg sends the background process group of the process pname a SIGCONT signal.\n\n");
        printf("Options :\n");
        printf("        --help : prints this help and exits\n");
	return 0;
    }
    else{
	//if(search_env(path_name) < 0) {fprintf(stderr,"error : environment search\n"); return -1;}
	//printf("%s\n",path_name);  //check
	int process_id = Search_by_name(process_list, path_name, 0);
	if(process_id < 0) {fprintf(stderr,"error : no such process\n"); return 0;}
	pid_t pgid = getpgid( process_id );
	if(pgid < 0) {fprintf(stderr,"error : getpgid\n"); return(-1);}
	
	if( shell_GID == pgid )
	{
		if(kill( process_id , SIGCONT ) < 0) {fprintf(stderr,"error : kill\n"); return(-1);}
	}
	else
	{
		if(kill( -pgid , SIGCONT ) < 0) {fprintf(stderr,"error : kill\n"); return(-1);}
	}

	return 0;
    }
}

int builtin_fg1(char *path_name, list *process_list)
{
    int wstatus;
    if(strcmp(path_name,"--help") == 0){
        printf("Synopsis : fg pname\n");
        printf("           fg --help\n\n");
        printf("Description :\nfg brings the process group of the process designated by pname to the foreground.\n\n");
        printf("Options :\n");
        printf("        --help : prints this help and exits\n");
	return 0;
    }
    else{
	//if(search_env(path_name) < 0) {fprintf(stderr,"error : environment search\n"); return -1;}
	//printf("%s\n",path_name);  //check
	int process_id = Search_by_name(process_list, path_name, 1);
	if(process_id < 0) {fprintf(stderr,"error : no such process\n"); return 0;}

	int current_job = Search_by_pid(process_list, process_id);   //jid of the current job
	change_status(process_list, current_job, 1);  //all jobs having same jid are given status 1

	pid_t pgid = getpgid( process_id );
	//printf("%d\n",process_id);
	if(pgid < 0) {fprintf(stderr,"error : getpgid\n"); return(-1);}
	pid_t shell_GID = getpgid(0);
	struct termios term_in;
	tcgetattr(STDIN_FILENO,&term_in);
	tcsetpgrp(STDIN_FILENO,pgid);
	if(shell_GID == pgid)
	{
		if(kill( process_id , SIGCONT ) < 0) {fprintf(stderr,"error : kill\n"); return(-1);}
		if( waitpid(-pgid,&wstatus,WUNTRACED) < 0 ) {fprintf(stderr,"error : wait\n"); return(-1);}
	}
	else
	{
		if(kill( -pgid , SIGCONT ) < 0) {fprintf(stderr,"error : kill\n"); return(-1);}
		if( waitpid(pgid,&wstatus,WUNTRACED) < 0 ) {fprintf(stderr,"error : wait\n"); return(-1);}
	}

	//restore shell------------------------------------------
	if( WIFSTOPPED(wstatus) )
	{
		fprintf(stderr,"%d stopped by signal\n",process_id);
		change_status(process_list,current_job,0);
	}
	tcsetpgrp(STDIN_FILENO,shell_GID);
	tcsetattr(STDIN_FILENO,TCSADRAIN,&term_in);
	//-------------------------------------------------------
	return 0;
    }
}

int fg_wrapper(char *path_name, list *process_list)
{
	if(builtin_fg1(path_name, process_list) < 0) fprintf(stderr,"error : fg\n");
	return 0;
}
//---------------------------------------------------------------------------------------------------

int builtin_setenv(const char **argv, const char *in_fname, const char *out_fname, int out_cond){
    int dup2_st, env_st;

    if(strcmp(in_fname,"stdin") != 0){
	if(my_file_dup((char *)in_fname,0,0) < 0) return -1;
    }
    if(strcmp(out_fname,"stdout") != 0){
	    if(out_cond == 1)
		{if(my_file_dup((char *)out_fname,1,1) < 0) return -1;}
	    else
	    	{if(my_file_dup((char *)out_fname,2,1) < 0) return -1;}
    }

    if(strcmp(argv[1],"--help") == 0){
        if(argv[2] != NULL) {fprintf(stderr,"error : setenv : invalid syntax\n"); return -1;}
        printf("Synopsis : setenv [option] variable_name value\n");
        printf("           setenv --help\n\n");
        printf("Description :\nsetenv sets the value of the environment variable given by 'variable_name' to the value specified by 'value'.");
        printf(" If -f option is set then this is done forcibly, otherwise the value of an existing variable is not changed. ");
        printf("If it is a new variable, it is added to the list of environment variables with the value set as 'value'.\n\n");
        printf("Options :\n");
        printf("        -f : forcibly sets the value\n");
        printf("        --help : prints this help and exits\n");
    }
    else if(strcmp(argv[1],"-f") == 0){
        if(argv[2] == NULL) {fprintf(stderr,"error : setenv : variable name absent\n"); return -1;}
        if(argv[3] == NULL) {fprintf(stderr,"error : setenv : value absent\n"); return -1;}
        if(argv[4] != NULL) {fprintf(stderr,"error : setenv : invalid syntax\n"); return -1;}
        env_st = setenv(argv[2],argv[3],1);
        if(env_st < 0) {fprintf(stderr,"error : setenv\n"); return -1;}
    }
    else{
        if(argv[1] == NULL) {fprintf(stderr,"error : setenv : variable name absent\n"); return -1;}
        if(argv[2] == NULL) {fprintf(stderr,"error : setenv : value absent\n"); return -1;}
        if(argv[3] != NULL) {fprintf(stderr,"error : setenv : invalid syntax\n"); return -1;}
        int env_st = setenv(argv[1],argv[2],0);
        if(env_st < 0) {fprintf(stderr,"error : setenv\n"); return -1;}
    }
    //strcpy(PWD,path_name);
    return 0;
}

int builtin_setenv1(const char **argv){
    int env_st;

    if(strcmp(argv[1],"--help") == 0){
        if(argv[2] != NULL) {fprintf(stderr,"error : setenv : invalid syntax\n"); return -1;}
        printf("Synopsis : setenv [option] variable_name value\n");
        printf("           setenv --help\n\n");
        printf("Description :\nsetenv sets the value of the environment variable given by 'variable_name' to the value specified by 'value'.");
        printf("If -f option is set then this is done forcibly, otherwise the value of an existing variable is not changed. If");
        printf("If it is a new variable, it is added to the list of environment variables with the value set as 'value'.\n\n");
        printf("Options :\n");
        printf("        -f : forcibly sets the value\n");
        printf("        --help : prints this help and exits\n");
    }
    else if(strcmp(argv[1],"-f") == 0){
        if(argv[2] == NULL) {fprintf(stderr,"error : setenv : variable name absent\n"); return -1;}
        if(argv[3] == NULL) {fprintf(stderr,"error : setenv : value absent\n"); return -1;}
        if(argv[4] != NULL) {fprintf(stderr,"error : setenv : invalid syntax\n"); return -1;}
        env_st = setenv(argv[2],argv[3],1);
        if(env_st < 0) {fprintf(stderr,"error : setenv\n"); return -1;}
    }
    else{
        if(argv[1] == NULL) {fprintf(stderr,"error : setenv : variable name absent\n"); return -1;}
        if(argv[2] == NULL) {fprintf(stderr,"error : setenv : value absent\n"); return -1;}
        if(argv[3] != NULL) {fprintf(stderr,"error : setenv : invalid syntax\n"); return -1;}
        int env_st = setenv(argv[1],argv[2],0);
        if(env_st < 0) {fprintf(stderr,"error : setenv\n"); return -1;}
    }
    //strcpy(PWD,path_name);
    return 0;
}

int setenv_wrapper(const char **argv, const char *in_fname, const char *out_fname, int out_cond){
    int temp_in = dup(0);
    if(temp_in < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int temp_out = dup(1);
    if(temp_out < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int env_st = builtin_setenv(argv, in_fname, out_fname, out_cond);
    if(env_st < 0) {fprintf(stderr,"error : builtin_setenv\n"); return -1;}
    int dup2_st = dup2(temp_in,0);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    dup2_st = dup2(temp_out,1);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    close(temp_in);
    close(temp_out);

    return 0;
}

int builtin_jobs(char *path_name,list *process_list, const char *in_fname, const char *out_fname, int out_cond){
    int dup2_st;

    if(strcmp(in_fname,"stdin") != 0){	
	if(my_file_dup((char *)in_fname,0,0) < 0) return -1;
    }
    if(strcmp(out_fname,"stdout") != 0){
	    if(out_cond == 1)
		{if(my_file_dup((char *)out_fname,1,1) < 0) return -1;}
	    else
	    	{if(my_file_dup((char *)out_fname,2,1) < 0) return -1;}
    }
    if(strcmp(path_name,"--help") == 0){
        printf("Synopsis : jobs\n");
        printf("           jobs --help\n\n");
        printf("Description :\njobs shows all the existing background jobs.\n\n");
        printf("Options :\n");
        printf("        --help : prints this help and exits\n");
    }
    else{
        node *temp = process_list->head;
	int curr_state = 0, curr_job;
	while(temp != NULL)
	{
		if(curr_state == 0)
		{
			printf("%d ",temp->job_id);
			curr_job = temp->job_id;
			curr_state = 1;
		}
		else if(curr_state == 1)
		{
			printf("%s ",temp->p_name);
			temp = temp->next;
			curr_state = 2;
		}
		else
		{
			if(temp->job_id == curr_job)
			{
				printf("| %s",temp->p_name);
				temp = temp->next;
			}
			else
			{
				printf("\n");
				curr_state = 0;
			}
		}
	}
	printf("\n");
    }
    //strcpy(PWD,path_name);
    return 0;
}

int builtin_jobs1(char *path_name,list *process_list){
    if(strcmp(path_name,"--help") == 0){
        printf("Synopsis : jobs\n");
        printf("           jobs --help\n\n");
        printf("Description :\njobs shows all the existing background jobs.\n\n");
        printf("Options :\n");
        printf("        --help : prints this help and exits\n");
    }
    else{
        node *temp = process_list->head;
	int curr_state = 0, curr_job;
	while(temp != NULL)
	{
		if(curr_state == 0)
		{
			printf("%d ",temp->job_id);
			curr_job = temp->job_id;
			curr_state = 1;
		}
		else if(curr_state == 1)
		{
			printf("%s ",temp->p_name);
			temp = temp->next;
			curr_state = 2;
		}
		else
		{
			if(temp->job_id == curr_job)
			{
				printf("| %s",temp->p_name);
				temp = temp->next;
			}
			else
			{
				printf("\n");
				curr_state = 0;
			}
		}
	}
	printf("\n");
    }
    //strcpy(PWD,path_name);
    return 0;
}

int jobs_wrapper(char *path_name,list *process_list, const char *in_fname, const char *out_fname, int out_cond){
    int temp_in = dup(0);
    if(temp_in < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int temp_out = dup(1);
    if(temp_out < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int env_st = builtin_jobs(path_name,process_list,in_fname,out_fname, out_cond);
    if(env_st < 0) {fprintf(stderr,"error : builtin_jobs\n"); return -1;}
    int dup2_st = dup2(temp_in,0);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    dup2_st = dup2(temp_out,1);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    close(temp_in);
    close(temp_out);

    return 0;
}

int builtin_unsetenv(const char *path_name, const char *in_fname, const char *out_fname, int out_cond){
    int dup2_st;
    if(strcmp(in_fname,"stdin") != 0){	
	if(my_file_dup((char *)in_fname,0,0) < 0) return -1;
    }
    if(strcmp(out_fname,"stdout") != 0){
	    if(out_cond == 1)
		{if(my_file_dup((char *)out_fname,1,1) < 0) return -1;}
	    else
	    	{if(my_file_dup((char *)out_fname,2,1) < 0) return -1;}
    }
    if(strcmp(path_name,"--help") == 0){
        printf("Synopsis : unsetenv variable_name\n");
        printf("           unsetenv --help\n\n");
        printf("Description :\nunsetenv removes the variable given by 'variable_name' from the environment.\n\n");
        printf("Options :\n");
        printf("        --help : prints this help and exits\n");
    }
    else{
        int env_st = unsetenv(path_name);
        if(env_st < 0) {fprintf(stderr,"error : unsetenv : variable name does not exist\n"); return -1;}
    }
    //strcpy(PWD,path_name);
    return 0;
}

int builtin_unsetenv1(const char *path_name){
    if(strcmp(path_name,"--help") == 0){
        printf("Synopsis : unsetenv variable_name\n");
        printf("           unsetenv --help\n\n");
        printf("Description :\nunsetenv removes the variable given by 'variable_name' from the environment.\n\n");
        printf("Options :\n");
        printf("        --help : prints this help and exits\n");
    }
    else{
        int env_st = unsetenv(path_name);
        if(env_st < 0) {fprintf(stderr,"error : unsetenv : variable name does not exist\n"); return -1;}
    }
    //strcpy(PWD,path_name);
    return 0;
}

int unsetenv_wrapper(const char *path_name, const char *in_fname, const char *out_fname, int out_cond){
    int temp_in = dup(0);
    if(temp_in < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int temp_out = dup(1);
    if(temp_out < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int env_st = builtin_unsetenv(path_name,in_fname,out_fname, out_cond);
    if(env_st < 0) {fprintf(stderr,"error : builtin_unsetenv\n"); return -1;}
    int dup2_st = dup2(temp_in,0);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    dup2_st = dup2(temp_out,1);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    close(temp_in);
    close(temp_out);

    return 0;
}

int builtin_cd(const char *path_name, const char *in_fname, const char *out_fname, int out_cond){
    int dup2_st;
    if(strcmp(in_fname,"stdin") != 0){	
	if(my_file_dup((char *)in_fname,0,0) < 0) return -1;
    }
    if(strcmp(out_fname,"stdout") != 0){
	    if(out_cond == 1)
		{if(my_file_dup((char *)out_fname,1,1) < 0) return -1;}
	    else
	    	{if(my_file_dup((char *)out_fname,2,1) < 0) return -1;}
    }
    int cd_st = chdir(path_name);
    if(cd_st < 0) {fprintf(stderr,"error : chdir\n"); return -1;}
    //strcpy(PWD,path_name);
    return 0;
}


int cd_wrapper(const char *path_name, const char *in_fname, const char *out_fname, int out_cond){
    int temp_in = dup(0);
    if(temp_in < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int temp_out = dup(1);
    if(temp_out < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int cd_st = builtin_cd(path_name,in_fname,out_fname, out_cond);
    if(cd_st < 0) {fprintf(stderr,"error : builtin_cd\n"); return -1;}
    int dup2_st = dup2(temp_in,0);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    dup2_st = dup2(temp_out,1);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    close(temp_in);
    close(temp_out);

    int size = 100;
    char *curr_dir = (char *)malloc(size * sizeof(char));
    curr_dir = getcwd(curr_dir,size);
    while(curr_dir == NULL)
    {
	    size = size * 2;
	    curr_dir = (char *)malloc(size * sizeof(char));
	    curr_dir = getcwd(curr_dir,size);
    }
    int env_st = setenv("PWD",curr_dir,1);
    if(curr_dir != NULL) free(curr_dir);
    if(env_st < 0) {fprintf(stderr,"error : setenv\n"); return -1;}
    return 0;
}

int cd_wrapper1(const char *path_name){
    int cd_st = chdir(path_name);
    if(cd_st < 0) {fprintf(stderr,"error : chdir\n"); return -1;}

    int size = 100;
    char *curr_dir = (char *)malloc(size * sizeof(char));
    curr_dir = getcwd(curr_dir,size);
    while(curr_dir == NULL)
    {
	    size = size * 2;
	    curr_dir = (char *)malloc(size * sizeof(char));
	    curr_dir = getcwd(curr_dir,size);
    }
    int env_st = setenv("PWD",curr_dir,1);
    if(curr_dir != NULL) free(curr_dir);
    if(env_st < 0) {fprintf(stderr,"error : setenv\n"); return -1;}
}
