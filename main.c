#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include "queue.h"

int concurrency, no_of_runningJobs = 0;
int max_core = 8;
job JOBS[100];
queue *JOBQ;

// function to complete individual job
void *complete_job(void *arg)
{
    job* jp = (job*) arg;
    char** args;
    pid_t pid;
    int status;

    /* Mark job as working and record start time */
    ++no_of_runningJobs;
    jp->job_status = "running";
    jp->start_time = current_datetime_str();

    /* Simulate some work before forking */
    sleep(3);

    /* Fork a child process to execute the job */
    pid = fork();
    if (pid == -1) {
        /* Error: fork failed */
        fprintf(stderr, "Error: process fork failed\n");
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        /* Child process */
        dup2(open_log(jp->fnout), STDOUT_FILENO); /* Redirect job stdout */
        dup2(open_log(jp->fnerr), STDERR_FILENO); /* Redirect job stderr */
        args = get_args(jp->command);
        execvp(args[0], args);
        /* Error: execvp failed */
        fprintf(stderr, "Error: command execution failed for \"%s\"\n", args[0]);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else {
        /* Parent process */
        if (waitpid(pid, &status, WUNTRACED) == -1) {
            /* Error: waitpid failed */
            fprintf(stderr, "Error: waitpid failed\n");
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        /* Record job exit status and completion time */
        jp->exit_status = status;
        jp->job_status = "complete";
        jp->stop_time = current_datetime_str();
        if (!WIFEXITED(status)) {
            /* Error: child process did not terminate normally */
            fprintf(stderr, "Child process %d did not terminate normally!\n", pid);
        }
    }

    /* Decrement number of working jobs */
    --no_of_runningJobs;
    return NULL;
}

// function to complete jobs continuously
void *complete_jobs()
{
    job *jp; // Pointer to a job.

    no_of_runningJobs = 0;
    // Infinite loop to process jobs from the queue.
    while(1)
    {
        // Check if there are jobs in the queue and if the maximum number of
        // concurrent threads hasn't been reached yet.
        if (JOBQ->count > 0 && no_of_runningJobs < concurrency)
        {
            // Get the next job from the queue.
            jp = queue_delete(JOBQ);

            // Create a new thread to complete the job.
            pthread_create(&jp->thread_id, NULL, complete_job, jp);

            // Detach the new thread to free its resources when it finishes.
            pthread_detach(jp->thread_id);
        }
        sleep(1); // Wait for one second before checking again.
    }
    return NULL; // Return NULL when done.    
}


int main(int argc, char **argv)
{
    // Declare a variable `thread_id` of type `pthread_t`, which will be used to store the ID of a thread
    pthread_t thread_id; 

    // Check if the number of command line arguments passed to the program is 2. If not, print usage instructions and exit.
    if (argc != 2) {
        printf("Usage: %s CONCURRENCY\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Convert the command line argument for concurrency to an integer
    concurrency = atoi(argv[1]);

    // If the value of concurrency is less than 1, set it to 1. If it is greater than 8, set it to 8.
    if (concurrency < 1) {
        concurrency = 1;
    } else if (concurrency > max_core) {
        concurrency = max_core;
    }

    // Print the value of concurrency
    printf("Concurrency: %d\n\n", concurrency);

    // Allocate memory for a string that will store the name of the log file, and create the file
    char *log_filename = malloc(strlen(argv[0]) + 5);
    sprintf(log_filename, "%s.err", argv[0]);
    int log_file_descriptor = open(log_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    // If the log file cannot be created, print an error and exit.
    if (log_file_descriptor == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Redirect standard error output to the log file
    if (dup2(log_file_descriptor, STDERR_FILENO) == -1) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    close(log_file_descriptor);

    // Initialize a job queue with a maximum size of 100
    JOBQ = queue_init(100);

    // Create a new thread that will execute the `complete_jobs` function
    if (pthread_create(&thread_id, NULL, complete_jobs, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    int job_count = 0;
    char input_line[1000];

    // Print the list of available commands
    printf("Enter a command: \n");
    printf("'submit program [arguments]': create and run a job.\n"
        "'showjobs': list all jobs that are currently waiting or running.\n"
        "'submithistory': list all jobs that were completed.\n"
        "'Ctrl+D' or 'Ctrl+C': exit.\n");
    
    printf("\nEnter Command> ");

    // Read input line by line until EOF or error occurs
    while (read_line(input_line, 1000) != -1) {

        // Extract the first word of the input as the command keyword
        char* keyword = strtok(get_copy(input_line), " \t\n\r\x0b\x0c");

        // Check if the keyword matches a known command and take action accordingly
        if (keyword != NULL) {

            // Handle the 'submit' command to create a new job
            if (strcmp(keyword, "submit") == 0) {
                if (job_count >= 100) {
                    printf("No of jobs are exceeding.\n");
                } else if (JOBQ->count >= JOBQ->size) {
                    printf("Job queue full, wait to complete some jobs.\n");
                } else {
                    // Extract the rest of the input as the job command and create a new job
                    char* command = left_strip(strstr(input_line, "submit") + 6);
                    JOBS[job_count] = create_job(command, job_count);
                    queue_insert(JOBQ, JOBS + job_count);
                    printf("Job %d added to the queue\n", job_count + 1);
                    job_count++;
                }
            }
            // Handle the 'showjobs' and 'submithistory' commands to list jobs
            else if (strcmp(keyword, "showjobs") == 0) {
                showjobs(JOBS, job_count);
            }
            else if(strcmp(keyword, "submithistory") == 0) {
                submithistory(JOBS, job_count);
            }
        }
        printf("Enter Command> ");
    }

    // Terminate the program upon reaching EOF
    kill(0, SIGINT);

    // Exit the program with a success status code
    exit(EXIT_SUCCESS);

}

