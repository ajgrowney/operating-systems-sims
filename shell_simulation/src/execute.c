/**
 * @file execute.c
 *
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

#include "execute.h"

#include <stdio.h>
#include<stdlib.h>
#include "quash.h"
#include "deque.h"

// Remove this and all expansion calls to it
/**
 * @brief Note calls to any function that requires implementation
 */
#define IMPLEMENT_ME()                                                  \
  fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)


//variable Used to initialize the job queue
bool initialized = false;

//pipes are used for interprocess communication
static int pipes[2][2];

//Referencing the deque.h document when creating a queue of type pid_t
IMPLEMENT_DEQUE_STRUCT(pid_queue, pid_t);
IMPLEMENT_DEQUE(pid_queue, pid_t);
//The pid_queue we will use throughout is called pid_q
pid_queue pid_q;


//
// Define the Job structure using a pid_queue from above
//
typedef struct Job{
  int job_id;
  char* cmd_job;
  pid_queue pid_q;
  pid_t pid;
} Job;

//
//Referencing the deque.h implementing a job queue of type Job
//
IMPLEMENT_DEQUE_STRUCT(job_queue, struct Job);
IMPLEMENT_DEQUE(job_queue, struct Job);
//The job_queue we will use throughout the project is called job_q
job_queue job_q;
int job_number = 1;

//The destroy_job function will be used to free cmd_job pointer to the heap allocation
void destroy_job(Job j) {
  free(j.cmd_job);
  destroy_pid_queue(&j.pid_q);
}

/***************************************************************************
 * Interface Functions
 ***************************************************************************/

// Return a string containing the current working directory.
char* get_current_directory(bool* should_free) {
  // TODO: Get the current working directory. This will fix the prompt path.
  // HINT: This should be pretty simple

  // Changed this to true since we use malloc to allocate memory on the heap
  *should_free = true;

  char* current = malloc(1024);

  //Call getcwd to pass the current working directory to the string current
  getcwd(current,1024);

  return current;
}

// Returns the value of an environment variable env_var
const char* lookup_env(const char* env_var) {

  //Create a char pointer to take in the environment variable using the getenv call
  const char* str = getenv(env_var);
  return str;
}

// Check the status of background jobs
void check_jobs_bg_status() {

  //Create a job  and pid that will check the status of mulitple background jobs as it iterates through the loop
  struct Job current_job;
  pid_t m_front;

  //Save the number of jobs that are currently in the job queue
  int num_of_jobs = length_job_queue(&job_q);
  if(num_of_jobs == 0) {
    return;
  }

  //Iterate through the job queue and check on their statuses
  for(int i = 0; i < num_of_jobs;i++){
    current_job = pop_front_job_queue(&job_q);

    int num_of_pids = length_pid_queue(&current_job.pid_q);

    m_front = peek_front_pid_queue(&current_job.pid_q);

    for(int num = 0; num < num_of_pids; num++){

      pid_t current_pid = pop_front_pid_queue(&current_job.pid_q);
      int status = false;

      if(waitpid(current_pid,&status,WNOHANG)==0){
        push_back_pid_queue(&current_job.pid_q,current_pid);
      }
    }

    if(is_empty_pid_queue(&current_job.pid_q)){
      print_job_bg_complete(current_job.job_id, m_front, current_job.cmd_job);
      destroy_job(current_job);
    }
    else{
      push_back_job_queue(&job_q,current_job);
    }
  }
}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
void print_job(int job_id, pid_t pid, const char* cmd) {
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
void print_job_bg_start(int job_id, pid_t pid, const char* cmd) {
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
void print_job_bg_complete(int job_id, pid_t pid, const char* cmd) {
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd) {
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char* exec = cmd.args[0];
  char** args = cmd.args;

  //Basic run generic command that uses execvp to search through system binaries

  execvp(exec,args);

  perror("ERROR: Failed to execute program");
}

// Print strings
void run_echo(EchoCommand cmd) {
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char** str = cmd.args;

  // DONE: Implement echo
  int i=0;
  //
  // Iterate through the char* array to print each part of the echo command
  //
  while(str[i]!=NULL){
  	printf("%s ",str[i]);
  	i++;
  }
  printf("\n");
  // Flush the buffer before returning
  fflush(stdout);
}

// Sets an environment variable
void run_export(ExportCommand cmd) {
  // Write an environment variable
  const char* env_var = cmd.env_var;
  const char* val = cmd.val;

  //Use set env with a final parameter of 1 to overwrite current environment variables
  setenv(env_var,val,1);

}

// Changes the current working directory
void run_cd(CDCommand cmd) {
  // Get the directory name
  const char* dir = cmd.dir;
  // Check if the directory is valid
  if (dir == NULL) {
    perror("ERROR: Failed to resolve path");
    return;
  }

  // Use chdir to change the directory to the value dir
  if (chdir(dir) !=0) {
    perror("ERROR: Directory change failed");
    return;
  }

  //Use a custom "OLD_PWD" to update the old path to working directory
  if(setenv("OLD_PWD",lookup_env("PWD"),1) != 0) {
    perror("ERROR: Failed to set OLD_PWD");
    return;
  }
  else if(setenv("PWD",dir,1) != 0) {
    perror("ERROR: Failed to set OLD_PWD");
    return;
  }
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd) {
  int signal = cmd.sig;
  int job_id = cmd.job;

  //Take in the number of jobs in the job queue and iterate through killing all the processes
  int num_jobs = length_job_queue(&job_q);
  for(int i = 0; i < num_jobs; i++) {
	  Job tmp = pop_front_job_queue(&job_q);
	  if(tmp.job_id == job_id){
		  size_t num_processes = length_pid_queue(&tmp.pid_q);
		  for(int j = 1; j <= num_processes; j++){
			  int tmp_pid = pop_front_pid_queue(&tmp.pid_q);
			  kill(tmp_pid, signal);
			  push_back_pid_queue(&tmp.pid_q, tmp_pid);
		  }
	  }
	  push_back_job_queue(&job_q, tmp);
  }
}



// Prints the current working directory to stdout
void run_pwd() {
  char pwd[1024];
  getcwd(pwd,sizeof(pwd));
  printf("%s\n",pwd);
  // Flush the buffer before returning
  fflush(stdout);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs() {
  int number_of_jobs = length_job_queue(&job_q);
  for(int i=0; i < number_of_jobs;i++){
    Job current_job = pop_front_job_queue(&job_q);

    printf("[%d]\t#PID#\t%s\n", current_job.job_id, current_job.cmd_job);

    push_back_job_queue(&job_q, current_job);
  }

  // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void child_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case GENERIC:
    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:
    run_jobs();
    break;

  case EXPORT:
  case CD:
  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void parent_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:
  case ECHO:
  case PWD:
  case JOBS:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where needed
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder, int i) {
  // Read the flags field from the parser
  bool p_in  = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in  = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND; // This can only be true if r_out
                                               // is true

  //If the pipe out is true, create the pipe here
  if(p_out){
    pipe(pipes[i%2]);
  }
  //Fork a child process to handle piping
  pid_t child_pid;
  child_pid = fork();
  push_back_pid_queue(&pid_q,child_pid);
  if(child_pid==0){

    if(p_in){
      //From the pipe in, connect the pipe to standard in
      dup2(pipes[(i-1)%2][0],STDIN_FILENO);
      close(pipes[(i-1)%2][0]);
    }
    if(p_out){
      //From the pipe out, connect the pipe to std out
      dup2(pipes[i%2][1],STDOUT_FILENO);
      close(pipes[i%2][1]);
    }
    if(r_in){
      //Using redirect in, read from file and connect to std in
      FILE* file = fopen(holder.redirect_in, "r");
      dup2(fileno(file),STDIN_FILENO);
    }
    if(r_out){
      if(r_app){
        //Check for the redirect append, and append
        FILE* file = fopen(holder.redirect_out,"a");
        //Connect the file to std out
        dup2(fileno(file),STDOUT_FILENO);
      }else{
        //If not redirect append, just write
        FILE* file = fopen(holder.redirect_out,"w");
        //connect the file to std out
        dup2(fileno(file),STDOUT_FILENO);
      }
    }
    child_run_command(holder.cmd); // This should be done in the child branch of a fork
    exit(0);
  }else{
    if(p_out){
      close(pipes[(i)%2][1]);
    }
    parent_run_command(holder.cmd); // This should be done in the parent branch of

  }
}

// Run a list of commands
void run_script(CommandHolder* holders) {
  if(!initialized){
    //Use the dequeue implement construtor to initialize the job queue
    job_q = new_job_queue(1);
    //Change the boolean flag to true so when running script again, it won't create another job queue
    initialized=true;
  }
  //Create a new process id queue each time the script is run
  pid_q = new_pid_queue(1);
  if (holders == NULL)
    return;
  check_jobs_bg_status();

  //Check to see if the command was exit or end
  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC) {
    end_main_loop();
    return;
  }

  CommandType type;

  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i)
    create_process(holders[i],i);

  if (!(holders[0].flags & BACKGROUND)) {
    // Not a background Job
    // Wait for all processes under the job to complete
    while(!is_empty_pid_queue(&pid_q)){
      pid_t current_pid = pop_front_pid_queue(&pid_q);
      int current_status;
      waitpid(current_pid,&current_status,0);
    }
    destroy_pid_queue(&pid_q);
  }
  else {
    // A background job.
    // Push the new job to the job queue
    struct Job current_job;
    current_job.job_id = job_number;
    job_number = job_number+1;
    current_job.pid_q = pid_q;
    current_job.cmd_job = get_command_string();
    current_job.pid = peek_back_pid_queue(&pid_q);
    push_back_job_queue(&job_q,current_job);
    print_job_bg_start(current_job.job_id, current_job.pid, current_job.cmd_job);
  }
}
