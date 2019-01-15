#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

extern char ** get_args();

int contains(char* A[], char* str) {
  for (int i = 0; A[i]; i++) {
    if (!strcmp(A[i], str))
      return 1;
  }
  return 0;
}
 

char** subarray(char** args, int start, int end) {
  fprintf(stderr, "In Subarray Now\n");
  for (int i = 0; args[i] != NULL; i++) {
            printf ("Argument %d: %s\n", i, args[i]);
  }
  fprintf(stderr,"subarray w indicies %d, to %d\n",start,end);
  char** sub = (char**) calloc(end-start + 1, sizeof(char*));
  
  for (int i = start; i < end; i++) {
    fprintf(stderr,"Transferring '%s' at position %d\n", args[i], i);
    sub[i - start] = (char*) calloc(strlen(args[i]) + 1, sizeof(char));
    strcpy(sub[i - start], args[i]);
  }
  for (int i = 0; sub[i]; i++){
    printf("%s\n", sub[i]);
  }
  return sub;
}

void execute(char* command, char* args[]) {
  printf("executing command %s with arguments: \n", command);
  for (int i = 0; args[i]; i++) {
    printf("%s ",args[i]);
  }
  printf("\n");
  //  int pid = fork();
  //  if (pid) { //pid != 0                                                 
  //    waitpid(pid,NULL, 0);
  //  } else {
    int s = execvp(command, args);
    perror("Something has gone wrong!: \n");
    //  }
}

void stdinFromFile(char* command, char* args[], char* file) {
  printf("redirecting from file '%s'\n",file);
  //  int pid = fork();
  //  if (!pid){
    int fin = open(file, O_RDONLY);
    if (fin < 0) {
      fprintf(stderr,"There was a problem opening file '%s'\n", file);
    }
    dup2(fin, STDIN_FILENO);
    close(fin);
    execvp(command, args);
    //  } else {
    //    waitpid(pid, NULL, 0);
    //  }
}

void stdoutToFile(char* command, char* args[], char* file) {
  printf("redirecting to file '%s' \n",file);
  //  int pid = fork();
  //  if (!pid){
    int fout = open(file, O_WRONLY | O_CREAT);
    if (fout < 0) {
      fprintf(stderr, "There was a problem opening file '%s'\n", file);
    }
    dup2(fout, STDOUT_FILENO);
    close(fout);
    execvp(command, args);

    //  } else {
    //    waitpid(pid,NULL,0);
    //  }
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

int
main()
{
    int         i;
    char **     args;

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
	  int pid = fork();
	  if (pid) {
	    waitpid(pid, NULL, 0);
	  } else {
	    char** newArgs = args;
	    for (i = 0; args[i]; i++) {
	      printf("checking %s\n", args[i]);	   
	      if (!strcmp(args[i], "<")) {
		newArgs = subarray(args, 0, i);
		printf("i've discovered that '%s' is '<' at argument %d\n", args[i], i);
		printf("Here's the subarray:\n");
		for (int a = 0; a < i; a++) {
		  printf("%s\n", newArgs[a]);
		}
		printf("-----------------\n");
		printf("Opening %s\n", args[i+1]);
		redirect(args[i+1], STDIN_FILENO, O_RDONLY);
	      } else if (!strcmp(args[i], ">")) {
		printf("i've discovered that '%s' is '>' at argument %d\n", args[i], i);
		if (newArgs == args) {
		  newArgs = subarray(args, 0, i);
		  printf("Here's the subarray:\n");
		  for (i = 0; newArgs[i]; i++) {
		    printf("%s\n",newArgs[i]);
		  }
		}
		printf("let's redirect w file %s\n", args[i+1]);
		redirect(args[i+1], STDOUT_FILENO, O_WRONLY | O_CREAT);
	      }	   
	    }
	    //	    int plain = 1;
	    //	    for (i = 0; args[i]; i++) {
	    //	      if (!strcmp(args[i], "<") ||
	    //		  !strcmp(args[i], ">")){
	    //		plain = 0;
	    //	      }
	    //	    }
	    //	  printf("let's execute\n");
	    //	    if (plain) {
	      execute(args[0], newArgs);
	    // }
	  }
	}
	int save = dup(STDIN_FILENO);
	close(STDIN_FILENO);
	dup2(save, STDIN_FILENO);
	printf("here we go looping again\n");
    }
    printf("broke out of while\n");
    return 0;
}
