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
  for (int i = 0; args[i] != NULL; i++) {
            printf ("Argument %d: %s\n", i, args[i]);
  }
  printf("subarray w indicies %d, to %d\n",start,end);
  char** sub = (char**) calloc(end-start, sizeof(char*));
  for (int i = start; i < end; i++) {
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
  int pid = fork();
  if (pid) { //pid != 0                                                 
    waitpid(pid,NULL, 0);
  } else {
    int s = execvp(command, args);
    perror("Something has gone wrong!: \n");
  }
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
	  char** newArgs = args;
	  for (i = 0; args[i]; i++) {
	    printf("checking %s\n", args[i]);	   
	    if (!strcmp(args[i], "<")) {
	      newArgs = subarray(args, 0, i);
	      printf("Here's the subarray:\n");
	      for (int a = 0; a < i; a++) {
		printf("%s\n", newArgs[a]);
	      }
	      printf("-----------------\n");
	      printf("Opening %s\n", args[i+1]);
	      int fin = open(args[i+1],O_RDONLY);
	      if (fin < 0) {
		printf("there was a problem opening file '%s'\n", args[i+1]);
	      }
	      dup2(fin, STDIN_FILENO);
	      close(fin);
	      break;
	    } else if (!strcmp(args[i], ">")) {
	      newArgs = subarray(args, 0, i);
	      int fout = open(args[i+1], O_WRONLY);
	      dup2(fout, STDOUT_FILENO);
	      close(fout);
	      break;
	    }	   
	  }
	  printf("let's execute\n");
	  execute(args[0], newArgs);
	}
    }
}
