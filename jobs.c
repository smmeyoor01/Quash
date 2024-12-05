#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "jobs.h"

struct job jobs[MAX_JOBS]; 
int count = 0;

void add_job(const char* cmd, pid_t pid) {
    if (count < MAX_JOBS) {
        jobs[count].pid = pid;

        snprintf(jobs[count].cmd, sizeof(jobs[count].cmd), "%s", cmd);
        jobs[count].job_id = count + 1;
        count++;
    }
}

void exec_jobs() {
    for(int x = 0; x < count; x++){
        printf("%d    %d    %s\n", jobs[x].job_id, jobs[x].pid, jobs[x].cmd);
    }
}

int find_job(pid_t pid){
    for(int x = 0; x < count; x++){
        if(jobs[x].pid == pid){return x;}
    }
    return -1;
}

void kill_job(const char* args){
    int i = 0;

    if(args[0] == '%'){
        int job_id = atoi(args + 1);
        if(job_id > 0 && job_id <= count){
            i = job_id - 1;
        }

        else{
            printf("Error killing job\n");
            return;
        }
    }

    else{
        pid_t pid = atoi(args);

        i = find_job(pid);
    }

    if(kill(jobs[i].pid, SIGTERM) == 0){
        printf("Job with id [%d] and pid %d has been terminated\n", jobs[i].job_id, jobs[i].pid);

        for(int x = i; x < count - 1; x ++){
            jobs[x] = jobs[x+1];
            jobs[x].job_id = x + 1;
            }
    count--;
    }
    else{
        perror("Error with killing process\n");
    }
}