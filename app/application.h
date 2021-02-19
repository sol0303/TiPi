#ifndef __APPLICATION_H_
#define __APPLICATION_H_


typedef struct
{
    unsigned char is_valid;
    time_t dead_time;
    char msg[1024];
}ISSUE;

void idle_issue();
int issue_schedule();
void add_tip(char* msg);
void add_note(char* msg);
#endif

