#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

extern char ** get_args(); 

void dprint(char* str) {
  fprintf(stderr, "%s", str);
}

int arrLen(char** A) {
  int count = 0;
  while (A[count]) {
    fprintf(stderr, "arrLen checking out A[%d]:%s\n",count, A[count]);
    count++;
  }
  return count;
}

char** subarray(char** args, int start, int end) {
  fprintf(stderr, "In subarray from %d to %d\n", start, end);
  //  for (int i = 0; args[i] != NULL; i++) {
  //            printf ("Argument %d: %s\n", i, args[i]);
  //  }
  //  fprintf(stderr,"subarray w indicies %d, to %d\n",start,end);
  char** sub = (char**) calloc(end-start + 1, sizeof(char*));
  for (int i = start; i < end; i++) {    
    sub[i - start] = (char*) calloc(strlen(args[i]) + 1, sizeof(char));
    strcpy(sub[i - start], args[i]);
  }
  dprint("Subarray containing : \n");
  for (int i = 0; sub[i]; i++){
    fprintf(stderr,"%s\n", sub[i]);
  }
  dprint("END subarray\n");
  return sub;
}

void execute(char* command, char* args[]) {
  fprintf(stderr,"executing command [ %s ] with arguments: \n----------------------\n", command);
  for (int i = 0; args[i]; i++) {
    fprintf(stderr,"%s, ",args[i]);
  }
  fprintf(stderr,"\n-------------------\n");
  int s = execvp(command, args);
  fprintf(stderr,"%d: Something has gone wrong!: %d\n", getpid(), errno);
}

void pipe_output_stdout(int fd[]) {
  close(fd[0]);
  close(STDOUT_FILENO);
  dup2(fd[1], STDOUT_FILENO);
  close(fd[1]);
}

void pipe_input_stdin(int fd[]) {
  close(fd[1]);
  close(STDIN_FILENO);
  dup2(fd[0], STDIN_FILENO);
  close(fd[0]);
}

void redirect(char*filename, int fd, int flags,...) {
  int nfd = open(filename, flags);
  if (nfd < 0) {
    fprintf(stderr, "There was a problem opening file '%s' in redirect\nExiting...\n", filename);
    exit(1);
  }
  dup2(nfd, fd);
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


void isolateRun(char** args) {//, int fd[2]) {
  fprintf(stderr,"isolated args:\n------------------------- %d\n", getpid());
  for (int j = 0; args[j]; j++) {
    fprintf(stderr, "%s, ", args[j]);
  }
  fprintf(stderr, "\n------------------------- %d\n",getpid());
  //  int pid = fork();
  //  if (pid) {
  //    waitpid(pid, NULL, 0);
  //  } else {
   char** newArgs = args;
   for (int i = 0; args[i]; i++) {
     //     printf("checking %s\n", args[i]);	   
     if (!strcmp(args[i], "<")) {
       newArgs = subarray(args, 0, i);
       //       printf("------Subarray----\n");
       for (int a = 0; a < i; a++) {
	 printf("%s\n", newArgs[a]);
       }
       //       printf("-----------------\n");
       //       printf("Opening %s\n", args[i+1]);
       dprint("redirecting STDIN from  file\n");
       redirect(args[i+1], STDIN_FILENO, O_RDONLY);
     } else if (!strcmp(args[i], ">")) {	
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       dprint("redirecting STDOUT to file\n");
       redirect(args[i+1], STDOUT_FILENO, O_RDWR | O_CREAT);
     } else if (!strcmp(args[i], ">>")) {
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       
       dprint("redirecting STDOUT to file with append\n");
       redirect(args[i+1], STDOUT_FILENO, O_RDWR| O_CREAT | O_APPEND);       
     } else if (!strcmp(args[i], ">&")) {
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       dprint("redirecting STDOUT and Err to file\n");
       redirect2(args[i+1], STDOUT_FILENO, STDERR_FILENO, O_RDWR | O_CREAT);
     } else if (!strcmp(args[i], ">>&")) {
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       dprint("redirecting STDOUT and Err to file with append\n");
       redirect2(args[i+1], STDERR_FILENO, STDOUT_FILENO, O_RDWR | O_CREAT | O_APPEND);
     } else if (!strcmp(args[i], "|")) {
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       int fd[2] = {-1,-1};
       if (pipe(fd)) {
	 perror("Problem creating pipe\n");
       }
       //       dprint("redirecting STDOUT to pipe[1]\n");
       //       pipe_output_stdout(fd);
       int nextpid = fork();
       char** sub = subarray(args, i + 1, arrLen(args));
       if (1) {
       
	 
	 if (nextpid == 0) { //child
	   pipe_input_stdin(fd);
	   execute(sub[0], sub);
	 } else { //parent
	   pipe_output_stdout(fd);
	   if (fork() == 0) {
	     execute(args[0], newArgs);	   
	   } else {
	     wait(0);
	     wait(0);
	   }
	 }
       }
       //##################################

       if (0) { 
	 if (nextpid == 0) {
	   
	   execute(args[0], newArgs);
	   perror("something has gone wrong :( \n");
	 } else {	 
	   waitpid(nextpid, NULL, 0);
	   fprintf(stderr,"args has %d args? \n", arrLen(args));
	   dprint("redirecting pip[0] to STDIN in parent process\n");
	   pipe_input_stdin(fd);
	   //	   char** sub = subarray(args, i + 1, arrLen(args));
	   execute(sub[0], sub);
	   //       	 isolateRun(sub);
	   return;
	 }
       }
     }
   }
   execute(args[0], newArgs);
   //  } //if child
}

void forknRun(char** args) {
  fprintf(stderr, "Creating a new forked process\n");
  int pid = fork();
  if (pid == 0) {
    int fd[2] = {-1,-1};
    isolateRun(args);//, fd);
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
	      //	      printf("time to to execute all of those beautiful commands\n");
	      forknRun(args); //isolateRun(args,0);
	    } else if (!strcmp(args[i], "cd")) {  /* handles cd command  */
	      if (args[i+1] == NULL) {
		dprint("There is nothing after cd!\n");
		chdir(homeDirectory);
		args = &args[i+1];
		i = -1;
	      } else if (!strcmp(args[i+1], ";")) {
		dprint("There is a semicolon nothing after cd!\n");
		chdir(homeDirectory);
		args = &args[i+2];
		i = -1;
	      } else {		
		printf("You want me to change the directory to '%s'?\n", args[i+1]);
		chdir(args[i+1]);
		args = &args[i+2];
		i = -1;
		if (args[i+2] != NULL && !strcmp(args[i+2], ";")) {
		  printf("I've just cahnged the directory to '%s' and now I might need to execute another command after '%s'\n",args[i+1],args[i+2]);
		  args = &args[i+3];
		  i = -1;
		}
	      }      	
	    } else if (!strcmp(args[i], ";")) {
	      dprint("we've encountered a juug semicolon\n");
	      forknRun(subarray(args, 0, i));// isolateRun(subarray(args, 0, i), 0);
	      args = &args[i+1];
	      i = -1;
	    } // if args[i] == 'cd'	   
	  } //for i in args
	  if (args[0])
	    forknRun(args);//isolateRun(args, 0);
	  //	  printf("There's a new player on the field\n");
	} // else from args[0] == NULl
    }
    return 0;
}
