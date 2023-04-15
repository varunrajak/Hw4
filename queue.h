/* Header file for the simple circular queue example */
#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct job
{
    int job_id;        
    pthread_t thread_id;  
    char *command, *job_status;     
    int exit_status;      
    char *start_time, *stop_time;     
    char fnout[10]; /* filename where job stdout is redirected */
    char fnerr[10]; /* filename where job stderr is redirected */
} job;


/* job functions */
job create_job(char *command, int job_id);
void showjobs(job *jobs, int n);
void submithistory(job *jobs, int n);


/* I/O functions and other helpers */
int read_line(char *s, int n);
char *left_strip(char *s);
char *get_copy(char *s);
char *get_copy_until_newline(char *s);
char *current_datetime_str();
char **get_args(char *line);
int open_log(char *fn);

typedef struct _queue
{
    int size;     /* maximum size of the queue */
    job **buffer; /* queue buffer */
    int start;    /* index to the start of the queue */
    int end;      /* index to the end of the queue */
    int count;    /* number of elements in the queue */
} queue;

queue *queue_init(int n);
int queue_insert(queue *q, job *jp);
job *queue_delete(queue *q);
void queue_destroy(queue *q);

#endif