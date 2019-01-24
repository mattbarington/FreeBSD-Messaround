#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

extern char ** get_args(); 


/**
 * Prints '<error> on line <lineNum> to perror, providing information about a failure that occured as a result of a system call, and on what line of code that error occured.
 */
void printError(char* error, int lineNum) {
  char buf[100];
  for (int i = 0; i < sizeof(buf); i++) {
    buf[i] = 0;
  }
  sprintf(buf, "%s on line %d\n", error, lineNum);
  perror(buf);    
}


/**
 * Used to find the length of a NULL terminated char** array.
 * @param A - A NULL terminated array of char*.
 * @return - the length of the A.
 */
int arrLen(char** A) {
  int count = 0;
  while (A[count]) {
    count++;
  }
  return count;
}

/**
 * Returns a heap-allocated deep copy array of given array args from indicies [start, end).
 * @param args - the parents array, a subarray of which will be returned.
 * @param start - The first index in the subarray. Inclusively bounded.
 * @param end - The lsat index in the subarray. Exclusively bounded.
 * @return subarray allocated on the heap.
 */
char** subarray(char** args, int start, int end) {
  char** sub = (char**) calloc(end-start + 1, sizeof(char*));
  for (int i = start; i < end; i++) {    
    sub[i - start] = (char*) calloc(strlen(args[i]) + 1, sizeof(char));
    strcpy(sub[i - start], args[i]);
  }
  return sub;
}

/**
 * wrapper function for execvp() system call. Upon failure of execvp(), error will be printed.
 */
void execute(char* command, char* args[]) {
  int s = execvp(command, args);
  printError("Error executing command", __LINE__);
}

/**
 * Redirects standard output to the write end of the provided pipe.
 * @param fd[] - the file descriptor array that acts as a pipe.
 * reference source - https://stackoverflow.com/questions/20187734/c-pipe-to-stdin-of-another-program
 */
void pipe_output_stdout(int fd[]) {
  close(fd[0]);
  close(STDOUT_FILENO);
  int ret = dup2(fd[1], STDOUT_FILENO);
  if (ret < 0) {
    printError("Error", __LINE__);
  }
  close(fd[1]);
}

/**
 * Redirects standard output and standard error to the write end of the provided pipe.
 * @param fd[] - the file descriptor array that acts as a pipe.
 * reference source - https://stackoverflow.com/questions/20187734/c-pipe-to-stdin-of-another-program
 */
void pipe_output_stdout_and_stderr(int fd[]) {
  close(fd[0]);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  int ret = dup2(fd[1], STDOUT_FILENO);
  ret |= dup2(fd[1], STDERR_FILENO);
  if (ret < 0) {
    printError("Error", __LINE__);
  }
  close(fd[1]);
}

/**
 * Redirects standard input to the read end of the provided pipe.
 * @param fd[] - the file descriptor array that acts as a pipe.
 * reference source - https://stackoverflow.com/questions/20187734/c-pipe-to-stdin-of-another-program
 */
void pipe_input_stdin(int fd[]) {
  close(fd[1]);
  close(STDIN_FILENO);
  int ret = dup2(fd[0], STDIN_FILENO);
  if (ret < 0) {
    printError("Error", __LINE__);
  }
  close(fd[0]);
}

/**
 * Redirects the given file descriptor to the given filename. The intent is to redirect stdin, stdout, and stderr from or to a file.
 * @param filename - the name of the file which input or output will be redirected from or to.
 * @param fd - the file descriptor to be redirected.
 * @param flags - the flags to be passed into open().
 * reference source - https://stackoverflow.com/questions/14543443/in-c-how-do-you-redirect-stdin-stdout-stderr-to-files-when-making-an-execvp-or
 */
void redirect(char*filename, int fd, int flags,...) {
  int nfd = open(filename, flags);
  if (nfd < 0) {
    fprintf(stderr, "There was a problem opening file '%s' in fnct redirect on line %d\nExiting...\n", filename, __LINE__);
    exit(errno);
  }
  int ret = dup2(nfd, fd);
  if (ret < 0) {
    printError("Error", __LINE__);
  }
  close(nfd);
}

/**
 * Redirects the given file descriptor to the given filename. The intent is to redirect stdin, stdout, and stderr from or to a file. * This function comes in handy when redirecting two file descriptors to the same file, e.g. STDER & STDOUT --> file.
 * @param filename - the name of the file which input or output will be redirected from or to.
 * @param fd1 - the file descriptor to be redirected.
 * @param fd2 - another  descriptor to be redirected.
 * @param flags - the flags to be passed into open().
 * reference source - https://stackoverflow.com/questions/14543443/in-c-how-do-you-redirect-stdin-stdout-stderr-to-files-when-making-an-execvp-or
 */
void redirect2(char* filename, int fd1, int fd2, int flags,...) {
  int nfd = open(filename, flags);
  if (nfd < 0) {
    fprintf(stderr, "There was a problem opening file '%s' in fnct redirect on line %d\nExiting...\n", filename, __LINE__);
    exit(errno);
  }
  if (dup2(nfd, fd1) < 0 ||  dup2(nfd, fd2) < 0) {
    printError("Error", __LINE__);
  }  
  close(nfd);
}

/**
 * Modularized to setup redirection between processes and execute commands.
 * It is intended that args passed into isolateRun represent one cohesive command or multiple commands that use redirection to communicate with eachother. It is assumed that there will be no semicolons representing commands that ought to run in isolation from one-another, as that is handled in the main loop.
 * @param args - the commands and redirectives to be processes and executed.
 */
void isolateRun(char** args) {
   char** newArgs = args;
   for (int i = 0; args[i]; i++) {
     if (!strcmp(args[i], "<")) {
       newArgs = subarray(args, 0, i); /* newArgs is going to be used as arguments to the actual execvp call.
					  ex: args = ["ls", ">", "file"]. newArgs will be ["ls"]. */
       redirect(args[i+1], STDIN_FILENO, O_RDONLY);
     } else if (!strcmp(args[i], ">")) {	
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       redirect(args[i+1], STDOUT_FILENO, O_RDWR | O_CREAT); // redirecting STDOUT to file
     } else if (!strcmp(args[i], ">>")) {
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       redirect(args[i+1], STDOUT_FILENO, O_RDWR| O_CREAT | O_APPEND); //  redirecting STDOUT to file with append\n
     } else if (!strcmp(args[i], ">&")) {
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       redirect2(args[i+1], STDOUT_FILENO, STDERR_FILENO, O_RDWR | O_CREAT);  // redirecting STDOUT and Err to file
     } else if (!strcmp(args[i], ">>&")) {
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       redirect2(args[i+1], STDERR_FILENO, STDOUT_FILENO, O_RDWR | O_CREAT | O_APPEND); // redirecting STDOUT and Err to file with append\n
     } else if (!strcmp(args[i], "|") || !strcmp(args[i], "|&")) { //piping stdout or stderr AND stdout.
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       int fd[2];
       if (pipe(fd)) {
	 perror("Problem creating pipe\n");
       }
       int pid1 = fork();
       char** sub = subarray(args, i + 1, arrLen(args));
       	 
       if (pid1 == 0) { //child
	 pipe_input_stdin(fd);	
	 isolateRun(sub);
       } else { //parent
	 if (!strcmp(args[i], "|&")) { //  piping stderr too
	   pipe_output_stdout_and_stderr(fd);
	 } else {           // just piping stdout
	   pipe_output_stdout(fd);
	 }
	 int pid2 = fork();
	 if (pid2 == 0) {
	   execute(args[0], newArgs);	   
	 } else {	   
	   waitpid(pid2, NULL, 0);
	   close(fd[1]);
	   waitpid(pid1, NULL, 0);
	   exit(0);
	 } // if pid2
       } // if pid1
     } // if '|'
   } // for args
   execute(args[0], newArgs);
}

/**
 * Creates a new process to run a set of commands in a isolated environment.
 * @param args - the argument array to pass into isolateRun() and 'pretend' that it's the only command running
 */
void forknRun(char** args) {
  int pid = fork();
  if (pid == 0) {
    int fd[2] = {-1,-1};
    isolateRun(args);
  } else {	    
    waitpid(pid, NULL, 0);
  }
}

int
main()
{
    int         i;
    char **     args;
    char homeDirectory[100];
    getcwd(homeDirectory, sizeof(homeDirectory));
    while (1) {

	printf ("Command ('exit' to quit): ");
	args = get_args();
	
	for (i = 0; args[i] != NULL; i++) {
	    printf ("Argument %d: %s\n", i, args[i]);
	}
	if (args[0] == NULL) {
	    printf ("No arguments on line!\n");
	} else if ( !strcmp (args[0], "exit")) {
	    printf ("Exiting...\n");

	    break;
	} else {
	  for (i = 0; args[i]; i++) {
	    if (args[i] == NULL) {
	      forknRun(args); 
	    } else if (!strcmp(args[i], "cd")) {  /* handles cd command  */
	      if (args[i+1] == NULL) { //There is nothing after cd!
		chdir(homeDirectory);
		args = &args[i+1];
		i = -1;
	      } else if (!strcmp(args[i+1], ";")) { // There is a semicolon nothing after cd
		chdir(homeDirectory);
		args = &args[i+2];
		i = -1;
	      } else {  // changing to specified directory
		chdir(args[i+1]);
		args = &args[i+2];
		i = -1;
		if (args[i+2] != NULL && !strcmp(args[i+2], ";")) { // changed directory. Checking for more commands after some semicolon
		  args = &args[i+3];
		  i = -1;
		}
	      }      	
	    } else if (!strcmp(args[i], ";")) {
	      forknRun(subarray(args, 0, i));
	      args = &args[i+1];
	      i = -1;
	    } // if args[i] == 'cd'	   
	  } //for i in args
	  if (args[0])  // not strictly necessary, but prevents an execvp( NULL ).
	    forknRun(args);
	} // else from args[0] == NULl
    }
    return 0;
}
