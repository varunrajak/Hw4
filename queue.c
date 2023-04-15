#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include "queue.h"


// function to create a job with values in argument
job create_job(char *command, int job_id) {
    job new_job;
    new_job.job_id = job_id;
    new_job.command = malloc(sizeof(char) * (strlen(command) + 1));
    strcpy(new_job.command, command);

    // Set the initial status of the new job to "waiting"
    new_job.job_status = "waiting";

    // Set the exit status of the new job to -1 (indicating that the job has not yet completed)
    new_job.exit_status = -1;
    new_job.start_time = new_job.stop_time = NULL;

    // Generate file names for the output and error logs of the new job using the job ID
    sprintf(new_job.fnout, "%d.out", new_job.job_id);
    sprintf(new_job.fnerr, "%d.err", new_job.job_id);

    return new_job;
}

// function to list all process that are either currently waiting or running
void showjobs(job *jobs, int num_jobs)
{
    // If the job list is empty or null, return
    if (jobs == NULL || num_jobs == 0) {
        return;
    }
    
    printf("%-10s %-40s %-15s", "jobid","command","status");

    // Loop through the job list and print the details of the incomplete jobs
    for (int i = 0; i < num_jobs; i++) {
        // Check if the job is not completed, and print its details
        if (strcmp(jobs[i].job_status, "complete") != 0) {
            printf("\n%-10d %-40s %-15s", jobs[i].job_id, jobs[i].command, jobs[i].job_status);
        }
    }
    printf("\n\n");
}

// function to list all the processes that were executed by your job scheduler
void submithistory(job *jobs, int num_jobs)
{
    // If the job list is empty or null, return
    if (jobs == NULL || num_jobs == 0) {
        return;
    }

    printf("%-10s %-40s %-15s %-15s %-15s", "Job ID","command","starttime", "endtime", "status");

    // Loop through the job list and print the details of the incomplete jobs
    for (int i = 0; i < num_jobs; i++) {
        // Check if the job is completed, and print its details
        if (strcmp(jobs[i].job_status, "complete") == 0) {
            char *st;
            if(jobs[i].exit_status == 256){
                st = "success";
            }
            printf("\n%-10d %-40s %-25s %-25s %-15s", jobs[i].job_id, jobs[i].command, jobs[i].start_time, jobs[i].stop_time, st);
        }
    }
    printf("\n\n");
}

// Create the queue data structure and initialize it.
queue *queue_init(int n)
{
    queue *q = (queue *)malloc(sizeof(queue));
    q->size = n;
    q->buffer = malloc(sizeof(job *) * n);
    q->start = 0;
    q->end = 0;
    q->count = 0;

    return q;
}

/* insert an item into the queue, update the pointers and count, and
   return the no. of items in the queue (-1 if queue is null or full) */
int queue_insert(queue *q, job *jp)
{
    if ((q == NULL) || (q->count == q->size))
        return -1;

    q->buffer[q->end % q->size] = jp;
    q->end = (q->end + 1) % q->size;
    q->count++;

    return q->count;
}

/* delete an item from the queue, update the pointers and count, and 
   return the item deleted (-1 if queue is null or empty) */
job *queue_delete(queue *q)
{
    if ((q == NULL) || (q->count == 0))
        return (job *)-1;

    job *j = q->buffer[q->start];
    q->start = (q->start + 1) % q->size;
    --q->count;

    return j;
}

/* display the contents of the queue data structure */
// void queue_display(queue *q) {
// 	int i;
// 	if (q != NULL && q->count != 0) {
// 		printf("queue has %d elements, start = %d, end = %d\n", 
// 			q->count, q->start, q->end);
// 		printf("queue contents: ");
// 		for (i = 0; i < q->count; i++)
// 	    		printf("%d ", q->buffer[(q->start + i) % q->size]);
// 		printf("\n");
// 	} else
// 		printf("queue empty, nothing to display\n");
// }

/*Delete the queue data structure.*/
void queue_destroy(queue *q)
{
    free(q->buffer);
    free(q);
}

// function to read line character by character
int read_line(char *line, int max_length)
{
    int i = 0;
    char c;

    // Read characters one by one from standard input
    while (i < max_length - 1 && (c = getchar()) != EOF && c != '\n') {
        line[i] = c;
        i++;
    }
    // If the last character read was a newline, replace it with a null terminator
    if (c == '\n') {
        line[i] = '\0';
    }
    // If we reached the end of the file, return -1 to indicate an error
    if (c == EOF && i == 0) {
        return -1;
    }
    // Otherwise, return the length of the string read
    return i;
}

/* Return a pointer to the first non-whitespace character in s. */
char *left_strip(char *s)
{
    char *p = s;
    // loop till it got a character in s
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\x0b' || *p == '\x0c')
        ++p;
    return p;
}


char *get_copy(char *s)
{
    size_t len = strlen(s);
    char *copy = malloc(len + 1);
    if (copy != NULL) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}


char *get_copy_until_newline(char *s)
{
    int i = 0;
    while (s[i] != '\0' && s[i] != '\n') {
        i++;
    }
    char *copy = malloc(sizeof(char) * (i + 1));
    strncpy(copy, s, i);
    copy[i] = '\0';
    return copy;
}

// function to get current date and time as string
char *current_datetime_str()
{
    time_t tim = time(NULL);
    return get_copy_until_newline(ctime(&tim));
}

char **get_args(char *line)
{
    char *copy = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(copy, line);

    char *arg;
    char **args = malloc(sizeof(char *));
    int i = 0;
    while ((arg = strtok(copy, " \t")) != NULL)
    {
        args[i] = malloc(sizeof(char) * (strlen(arg) + 1));
        strcpy(args[i], arg);
        args = realloc(args, sizeof(char *) * (++i + 1));
        copy = NULL;
    }
    args[i] = NULL;
    return args;
}

/* Open a log file with the given filename and return its file descriptor. */
int open_log(char *fn)
{
    int fd;
    if ((fd = open(fn, O_CREAT | O_APPEND | O_WRONLY, 0755)) == -1)
    {
        fprintf(stderr, "Error: failed to open \"%s\"\n", fn);
        perror("open");
        exit(EXIT_FAILURE);
    }
    return fd;
}
