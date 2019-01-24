#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

extern char ** get_args(); 

void printError(char* error, int lineNum) {
  char buf[100];
  for (int i = 0; i < sizeof(buf); i++) {
    buf[i] = 0;
  }
  sprintf(buf, "%s on line %d\n", error, lineNum);
  perror(buf);    
}

void dprint(char* str) {
  fprintf(stderr, "%s", str);
}

int arrLen(char** A) {
  int count = 0;
  while (A[count]) {
    count++;
  }
  return count;
}

char** subarray(char** args, int start, int end) {
  char** sub = (char**) calloc(end-start + 1, sizeof(char*));
  for (int i = start; i < end; i++) {    
    sub[i - start] = (char*) calloc(strlen(args[i]) + 1, sizeof(char));
    strcpy(sub[i - start], args[i]);
  }
  return sub;
}

void execute(char* command, char* args[]) {
  int s = execvp(command, args);
  printError("Error executing command", __LINE__);
}

void pipe_output_stdout(int fd[]) {
  close(fd[0]);
  close(STDOUT_FILENO);
  int ret = dup2(fd[1], STDOUT_FILENO);
  if (ret < 0) {
    printError("Error", __LINE__);
  }
  close(fd[1]);
}

void pipe_input_stdin(int fd[]) {
  close(fd[1]);
  close(STDIN_FILENO);
  int ret = dup2(fd[0], STDIN_FILENO);
  if (ret < 0) {
    printError("Error", __LINE__);
  }
  close(fd[0]);
}

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

void redirect2(char* filename, int fd1, int fd2, int flags,...) {
  int nfd = open(filename, flags);
  if (nfd < 0) {
    fprintf(stderr, "There was a problem opening file '%s' in redirect\nExiting...\n", filename);
    exit(1);
  }
  dup2(nfd, fd1);
  dup2(nfd, fd2);
  close(nfd);
}


void isolateRun(char** args) {
   char** newArgs = args;
   for (int i = 0; args[i]; i++) {
     if (!strcmp(args[i], "<")) {
       newArgs = subarray(args, 0, i);
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
     } else if (!strcmp(args[i], "|")) {
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
	 pipe_output_stdout(fd);
	 int pid2 = fork();
	 if (pid2 == 0) {
	   execute(args[0], newArgs);	   
	 } else {	   
	   waitpid(pid2, NULL, 0);
	   close(fd[1]);
	   waitpid(pid1, NULL, 0);
	   return;
	 }
       }       
     }
   }
   execute(args[0], newArgs);
   //  } //if child
}

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
