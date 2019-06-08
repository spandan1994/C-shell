
#include "builtin.h"

//not yet fullproof, needs checking----------------------------------------------------------------
int builtin_fg1(const char *path_name)
{

    if(strcmp(path_name,"--help") == 0){
        printf("Synopsis : fg pid\n");
        printf("           fg --help\n\n");
        printf("Description :\nfg brings the process group of the process specified by pid to the foreground.\n\n");
        printf("Options :\n");
        printf("        --help : prints this help and exits\n");
	return 0;
    }
    else if(atoi(path_name) > 0){
	pid_t pgid = getpgid(atoi(path_name));
	if(pgid < 0) {fprintf(stderr,"error : getpgid\n"); return(-1);}
	pid_t shell_GID = getpgid(0);
	struct termios term_in;
	tcgetattr(STDIN_FILENO,&term_in);
	signal (SIGTTOU, SIG_IGN);
	signal (SIGTSTP, SIG_IGN);
	tcsetpgrp(STDIN_FILENO,pgid);
	if(kill( -(atoi(path_name)) , SIGCONT ) < 0) {fprintf(stderr,"error : kill\n"); return(-1);}

	//restore shell------------------------------------------
	if(waitpid(atoi(path_name),NULL,0) < 0) {fprintf(stderr,"error : wait\n"); return(-1);}
	tcsetpgrp(STDIN_FILENO,shell_GID);
	tcsetattr(STDIN_FILENO,TCSADRAIN,&term_in);
	//-------------------------------------------------------
	return 0;
    }
    //strcpy(PWD,path_name);
    printf("pid not valid\n");
    return 0;
}

int fg_wrapper(const char *path_name)
{
	if(builtin_fg1(path_name) < 0) fprintf(stderr,"error : fg\n");
	signal (SIGTTOU, SIG_DFL);
	signal (SIGTSTP, SIG_DFL);
	return 0;
}
//---------------------------------------------------------------------------------------------------

int builtin_setenv(const char **argv, const char *in_fname, const char *out_fname){
    int dup2_st, env_st;

    if(strcmp(in_fname,"stdin") != 0){
        int curr_in = open(in_fname,O_RDONLY);
        if(curr_in < 0) {fprintf(stderr,"error : open\n"); return -1;}
        dup2_st = dup2(curr_in,0);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
        close(curr_in);
    }
    if(strcmp(out_fname,"stdout") != 0){
        int curr_out = open(out_fname,O_CREAT|O_WRONLY|O_TRUNC,00600);
        if(curr_out < 0) {fprintf(stderr,"error : open\n"); return -1;}
        dup2_st = dup2(curr_out,1);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
        close(curr_out);
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

int setenv_wrapper(const char **argv, const char *in_fname, const char *out_fname){
    int temp_in = dup(0);
    if(temp_in < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int temp_out = dup(1);
    if(temp_out < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int env_st = builtin_setenv(argv, in_fname, out_fname);
    if(env_st < 0) {fprintf(stderr,"error : builtin_setenv\n"); return -1;}
    int dup2_st = dup2(temp_in,0);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    dup2_st = dup2(temp_out,1);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    close(temp_in);
    close(temp_out);

    return 0;
}

int builtin_unsetenv(const char *path_name, const char *in_fname, const char *out_fname){
    int dup2_st;

    if(strcmp(in_fname,"stdin") != 0){
        int curr_in = open(in_fname,O_RDONLY);
        if(curr_in < 0) {fprintf(stderr,"error : open\n"); return -1;}
        dup2_st = dup2(curr_in,0);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
        close(curr_in);
    }
    if(strcmp(out_fname,"stdout") != 0){
        int curr_out = open(out_fname,O_CREAT|O_WRONLY|O_TRUNC,00600);
        if(curr_out < 0) {fprintf(stderr,"error : open\n"); return -1;}
        dup2_st = dup2(curr_out,1);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
        close(curr_out);
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

int unsetenv_wrapper(const char *path_name, const char *in_fname, const char *out_fname){
    int temp_in = dup(0);
    if(temp_in < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int temp_out = dup(1);
    if(temp_out < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int env_st = builtin_unsetenv(path_name,in_fname,out_fname);
    if(env_st < 0) {fprintf(stderr,"error : builtin_unsetenv\n"); return -1;}
    int dup2_st = dup2(temp_in,0);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    dup2_st = dup2(temp_out,1);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    close(temp_in);
    close(temp_out);

    return 0;
}

int builtin_cd(const char *path_name, const char *in_fname, const char *out_fname){
    int dup2_st;

    if(strcmp(in_fname,"stdin") != 0){
        int curr_in = open(in_fname,O_RDONLY);
        if(curr_in < 0) {fprintf(stderr,"error : open\n"); return -1;}
        dup2_st = dup2(curr_in,0);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
        close(curr_in);
    }
    if(strcmp(out_fname,"stdout") != 0){
        int curr_out = open(out_fname,O_CREAT|O_WRONLY|O_TRUNC,00600);
        if(curr_out < 0) {fprintf(stderr,"error : open\n"); return -1;}
        dup2_st = dup2(curr_out,1);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
        close(curr_out);
    }
    int cd_st = chdir(path_name);
    if(cd_st < 0) {fprintf(stderr,"error : chdir\n"); return -1;}
    //strcpy(PWD,path_name);
    return 0;
}


int cd_wrapper(const char *path_name, const char *in_fname, const char *out_fname){
    int temp_in = dup(0);
    if(temp_in < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int temp_out = dup(1);
    if(temp_out < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int cd_st = builtin_cd(path_name,in_fname,out_fname);
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
