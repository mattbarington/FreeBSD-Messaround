#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

extern char ** get_args(); 

char** subarray(char** args, int start, int end) {
  //  for (int i = 0; args[i] != NULL; i++) {
  //            printf ("Argument %d: %s\n", i, args[i]);
  //  }
  //  fprintf(stderr,"subarray w indicies %d, to %d\n",start,end);
  char** sub = (char**) calloc(end-start + 1, sizeof(char*));
  
  for (int i = start; i < end; i++) {    
    sub[i - start] = (char*) calloc(strlen(args[i]) + 1, sizeof(char));
    strcpy(sub[i - start], args[i]);
  }
  //  for (int i = 0; sub[i]; i++){
  //    printf("%s\n", sub[i]);
  //  }
  return sub;
}

void execute(char* command, char* args[]) {
  //  printf("executing command %s with arguments: \n", command);
  //  for (int i = 0; args[i]; i++) {
  //    printf("%s ",args[i]);
  //  }
  int s = execvp(command, args);
  perror("Something has gone wrong!: \n");
}

int redirect(char*filename, int fd, int flags,...) {
  int saved = dup(fd);
  int nfd = open(filename, flags);
  if (nfd < 0) {
    fprintf(stderr, "There was a problem opening file '%s' in redirect\nExiting...\n", filename);
    exit(1);
  }
  dup2(nfd, fd);
  close(nfd);
  return saved;
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
	      //	      printf("checking %s\n", args[i]);	   
	      if (!strcmp(args[i], "<")) {
		newArgs = subarray(args, 0, i);
		//for (int a = 0; a < i; a++) {
		//		  printf("%s\n", newArgs[a]);
		//		}
		//		printf("-----------------\n");
		//		printf("Opening %s\n", args[i+1]);
		int saved_stdin = redirect(args[i+1], STDIN_FILENO, O_RDONLY);
	      } else if (!strcmp(args[i], ">")) {	
		if (newArgs == args) {
		  newArgs = subarray(args, 0, i);
		}
		int saved_stdout = redirect(args[i+1], STDOUT_FILENO, O_WRONLY | O_CREAT);
	      } else if (!strcmp(args[i], ">>")) {
		int saved_stdout = redirect(args[i+1], STDOUT_FILENO, O_WRONLY | O_CREAT | O_APPEND);
		if (newArgs == args) {
		  newArgs = subarray(args, 0, i);
		}		
	      }
	    }
	    execute(args[0], newArgs);
	  }
	}
	
    }
    printf("broke out of while\n");
    return 0;
}
