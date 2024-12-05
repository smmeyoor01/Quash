#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "jobs.h"
#include "pipe.h"

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_COUNT 64

void read_command(char* input){
   printf("[Quash]$ ");

   fgets(input, MAX_INPUT_SIZE, stdin);
   input[strcspn(input, "\n")] = 0;
}


void parse_command(char* input, char** args1, char** args2){
   
   char* ptr = strchr(input, '|');
   char* cmd1;
   char* cmd2;
   

   if(ptr != NULL){
    *ptr = '\0';
    cmd1 = input;
    cmd2 = ptr + 1;

    int arg_index = 0;
    char* one = strtok(cmd1, " \t");
    while(one != NULL && arg_index < MAX_ARG_COUNT - 1){
        args1[arg_index++] = one;
        one = strtok(NULL, " \t");
    }

    args1[arg_index] = NULL;

    arg_index = 0;
    one = strtok(cmd2, " \t");


    while(one != NULL && arg_index < MAX_ARG_COUNT - 1){
        args2[arg_index++] = one;
        one = strtok(NULL, " \t");
    }
    args2[arg_index] = NULL;

   }

   else{
    int arg_index = 0;
    char* one = strtok(input, " \t");
    while(one != NULL && arg_index < MAX_ARG_COUNT - 1){
        args1[arg_index++] = one;
        one = strtok(NULL, " \t");
    }

    args1[arg_index] = NULL;
    args2[0] = NULL;
   }
}



void check_jobs(){
   int s;
   pid_t pid;

   for(int x = 0; x < count; x++){
      pid = waitpid(jobs[x].pid, &s, WNOHANG);

      if(pid > 0){
         if(WIFEXITED(s)){
            printf("Job %s with pid %d and job id %d completed with status %d\n", jobs[x].cmd, jobs[x].pid, jobs[x].job_id, WEXITSTATUS(s));
         }
      
      for (int y = x; y < count -1; y++){
         jobs[y] = jobs[y + 1];
      }
      count--;
      x--;


      }
   }
}






void exec_command(char **args) {
    int background = 0;
    int redirect_i = -1;  // Input redirection
    int redirect_o = -1;  // Output redirection

    char *file_i = NULL;
    char *file_o = NULL;

    for (int x = 0; args[x] != NULL; x++) {
        if (strcmp(args[x], "&") == 0) {
            background = 1;
            args[x] = NULL;
        }
        else if (strcmp(args[x], ">") == 0) {
            file_o = args[x + 1];  // Get the output file
            args[x] = NULL;  // Stop at the redirection operator
        }
        else if (strcmp(args[x], "<") == 0) {
            file_i = args[x + 1];  // Get the input file
            args[x] = NULL;  // Stop at the redirection operator
        }
    }

    pid_t pid = fork();
    if (pid == 0) {  // Child process
        // Handle input redirection
        if (file_i != NULL) {
            redirect_i = open(file_i, O_RDONLY);
            if (redirect_i == -1) {
                perror("Error opening input file");
                exit(EXIT_FAILURE);
            }
            dup2(redirect_i, STDIN_FILENO);
            close(redirect_i);
        }

        // Handle output redirection
        if (file_o != NULL) {
            redirect_o = open(file_o, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (redirect_o == -1) {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
            dup2(redirect_o, STDOUT_FILENO);
            close(redirect_o);
        }

        // Execute the command
        if (execvp(args[0], args) == -1) {
            perror("Error executing command");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid > 0) {  // Parent process
        add_job(args[0], pid);
        if (!background) {
            waitpid(pid, NULL, 0);
        }
    } 
    else {  // Fork failed
        perror("Fork failed");
    }
    
    check_jobs();
}




int exec_built_in(char** args){

   for (int x = 0; args[x] != NULL; x++){
      if(strcmp(args[x], "<") == 0 || strcmp(args[x], ">") == 0){
         return 0;
      }
   }
   
   if(strcmp(args[0], "cd") == 0){
      if(args[1] == NULL || chdir(args[1]) != 0){
         perror("Yo the cd aint working");
      }
      return 1;
   }
   
   if(strcmp(args[0], "echo") == 0){
      for(int i = 1; args[i] != NULL; i++){
         if(args[i][0] == '$'){
            char* var = getenv(args[i] + 1);
            if(var){printf("%s ", var);}
            else{
               printf(" ");
            }
         }
         else{
            printf("%s", args[i]);
         }
      }
   printf("\n");
   return 1;
   }


   if(strcmp(args[0], "export") == 0){
      if(args[1] != NULL){
         char* name = strtok(args[1], "=");
         char* value = strtok(NULL, "=");
         if(name != NULL && value != NULL){
            setenv(name, value, 1);
         }
         else{perror("Yo your export isn't working");}
      }
      return 1;
   }

   if(strcmp(args[0], "jobs") == 0){
      exec_jobs();
      return 1;
   }

    if(strcmp(args[0], "kill") == 0){
        if (args[1] != NULL){
        kill_job(args[1]);
        }
        return 1;
    }




   return 0;
   
}




int main(){


char input[MAX_INPUT_SIZE];
char* args1[MAX_ARG_COUNT];
char* args2[MAX_ARG_COUNT];


while(1){
   check_jobs();
   read_command(input);
   parse_command(input, args1, args2);


   if (args2[0] != NULL) {
      exec_pipe(args1, args2);
      check_jobs();
   } else {
      if(exec_built_in(args1) == 0) {
         exec_command(args1);
      }
   }
   }
    return 0;
}
