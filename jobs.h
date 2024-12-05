#ifndef JOBS_H
#define JOBS_H
#include <sys/types.h>

#define MAX_JOBS 150





struct job {
    char cmd[1024];
    int job_id;          
    pid_t pid;           
};

extern struct job jobs[MAX_JOBS];
extern int count;

void add_job(const char* cmd, pid_t pid);
void exec_jobs();
int find_job(pid_t pid);
void kill_job(const char* args);

#endif