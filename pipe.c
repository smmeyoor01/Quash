#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void exec_pipe(char** args1, char** args2){

    int pipefd[2];
    pid_t pid1, pid2;

    if(pipe(pipefd) < 0){
       
        perror("Issue with fork");
        return;
    }

    pid1 = fork();
    if(pid1 < 0){
        perror("Issue with fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if(pid1 == 0){
        
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        execvp(args1[0], args1);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    pid2 = fork();

    if(pid2 < 0){
        perror("fork");
        return;
    }

    if(pid2 == 0){
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        execvp(args2[0], args2);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

}