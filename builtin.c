
#include "builtin.h"

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

int modify_pwd(){
    int fdpipe[2], dup2_st;
    char temp[100];
    int pipe_st = pipe(fdpipe);
    if(pipe_st < 0) {fprintf(stderr,"error : pipe\n"); return -1;}

    int status = fork();
    if(status == 0){
        dup2(fdpipe[1],1);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); exit(-1);}
        close(fdpipe[0]);
        close(fdpipe[1]);
        char *arglist[] = {"/bin/pwd",NULL};
        int exec_st = execv(arglist[0],arglist);
        if(exec_st < 0) {fprintf(stderr,"error : exec : pwd\n"); exit(-1);}
    }
    else{
        int wstatus;
        dup2_st = dup2(fdpipe[0],0);
        if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
        close(fdpipe[0]);
        close(fdpipe[1]);
        close(1);
        //temp = readline(NULL);
        scanf("%s",temp);
        int wait_st = waitpid(-1,&wstatus,0);
        if(wait_st < 0) {fprintf(stderr,"error : wait\n"); return -1;}
        if(WEXITSTATUS(wstatus) == -1) return -1;
        //scanf("%c",&t);
        int env_st = setenv("PWD",temp,1);
        if(env_st < 0) {fprintf(stderr,"error : setenv\n"); return -1;}
        return 0;
    }
}

int modify_pwd_wrapper(){
    int temp_in = dup(0);
    if(temp_in < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int temp_out = dup(1);
    if(temp_out < 0) {fprintf(stderr,"error : dup\n"); return -1;}
    int pwd_st = modify_pwd();
    if(pwd_st < 0) {fprintf(stderr,"error : modify_pwd\n"); return -1;}
    int dup2_st = dup2(temp_in,0);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    dup2_st = dup2(temp_out,1);
    if(dup2_st < 0) {fprintf(stderr,"error : dup2\n"); return -1;}
    close(temp_in);
    close(temp_out);
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

    int pwd_st = modify_pwd_wrapper();
    if(pwd_st < 0) return -1;
    return 0;
}

int cd_wrapper1(const char *path_name){
    int cd_st = chdir(path_name);
    if(cd_st < 0) {fprintf(stderr,"error : chdir\n"); return -1;}

    int pwd_st = modify_pwd_wrapper();
    if(pwd_st < 0) return -1;
    return 0;
}