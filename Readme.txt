## Description



In this project we have implemented a simple job scheduler that will execute non-interactive jobs. 

At any given time, only P jobs should be executing, and P is provided as an argument to your program. 

If you have more than P jobs submitted, then these additional jobs must wait until one of the P executing jobs are completed.





it redirects the output and error streams of each job to `<jobID>.out` and `<jobID>.err`, respectively.



### Approach



Program is running in threads and new processes.

we give divided task to threads.



### To Compile and run



Run the following command:



- "make"

- "./main <p>"  here p is a number between [1,8].





### Commands to input



- "submit program arguments"

  Example :
  - submit program ls 
  - submit sleep 10
  
  Create a new process to execute the program specified with any arguments and print a jobid to standard output.



- "showjobs"

  List all process that are either currently waiting or running (only those process that were started using the submit command). The output should include the jobid assigned to each job and the status of the jobs (whether it is running or waiting). If the job has completed, it will not be listed.



- "submithistory"

  List all the processes that were executed by your job scheduler, including the name of the job, the jobid that was assigned to it, the start time and end time of the job, and the status of the job (whether the job completed successfully or not).