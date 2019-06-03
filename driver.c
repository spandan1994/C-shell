//chdir does not change pwd variable. That must be done by you.
#include "execution.h"

PIPE_LINE command_seq;

int main(){

    int i,j,m,k;

    modify_pwd_wrapper(); //does not work here, why?

    /*int env_st = setenv("PWD","/home/spandan",1);
    if(env_st < 0) fprintf(stderr,"error : setenv\n");
    printf("%s\n",getenv("PWD"));*/

    for(k=0 ;  ; k++){
        printf("Enter number of simple commands : ");
        fflush(stdout);
        scanf("%d",&(command_seq.num_cmds));
        command_seq.arglists = (char ***)malloc(command_seq.num_cmds * sizeof(char **));

        for(i=0 ; i<command_seq.num_cmds ; i++){
            printf("arglist for command %d :\n",i);
            printf("Enter number of arguments : ");
            fflush(stdout);
            scanf("%d",&m);
            command_seq.arglists[i] = (char **)malloc((m+1) * sizeof(char *));
            for(j=0 ; j<m ; j++){
                printf("arg[%d] = ",j);
                fflush(stdout);
                command_seq.arglists[i][j] = (char *)malloc(100 * sizeof(char));
                scanf("%s",command_seq.arglists[i][j]);
            }
            command_seq.arglists[i][j] = NULL;
        }

        printf("Enter input filename : ");
        fflush(stdout);
        scanf("%s",command_seq.in_fname);

        printf("Enter output filename : ");
        fflush(stdout);
        scanf("%s",command_seq.out_fname);

        printf("Whether parent should wait (0/1) : ");
        fflush(stdout);
        scanf("%d",&(command_seq.background));

        for(i=0 ; i<command_seq.num_cmds ; i++){
            //if path for command i is not given and command i is not a builtin : actually this check is unnecessary
            int find_st = find_path(i,command_seq.arglists);
            if(find_st < 0) fprintf(stderr,"error : find_path\n");
            //printf("%s\n",arglists[i][0]);
        }


        int exec_st = exec_wrapper(&command_seq);
        if(exec_st < 0) fprintf(stderr,"error : exec_wrapper\n");
        free(command_seq.arglists);
    }


    printf("My name is Spandan Das %s\n",getenv("PWD"));

}
