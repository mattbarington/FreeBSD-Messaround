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
  fprintf(stderr,"executing command [ %s ] with arguments: \n----------------------\n", command);
  for (int i = 0; args[i]; i++) {
    fprintf(stderr,"%s, ",args[i]);
  }
  fprintf(stderr,"\n-------------------\n");
  int s = execvp(command, args);
  fprintf(stderr,"%d: Something has gone wrong!: %d\n", getpid(), errno);
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

void dprint(char* str) {
  fprintf(stderr, "%s", str);
}

void isolateRun(char** args) {
  fprintf(stderr,"isolated args:\n-------------------------\n");
  for (int j = 0; args[j]; j++) {
    fprintf(stderr, "%s, ", args[j]);
  }
  fprintf(stderr, "\n-------------------------\n");
  int pid = fork();
  if (pid) {
    waitpid(pid, NULL, 0);
  } else {
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
       redirect(args[i+1], STDIN_FILENO, O_RDONLY);
     } else if (!strcmp(args[i], ">")) {	
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       redirect(args[i+1], STDOUT_FILENO, O_RDWR | O_CREAT);
     } else if (!strcmp(args[i], ">>")) {
       redirect(args[i+1], STDOUT_FILENO, O_RDWR| O_CREAT | O_APPEND);
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }		
     } else if (!strcmp(args[i], ">&")) {
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       redirect2(args[i+1], STDOUT_FILENO, STDERR_FILENO, O_RDWR | O_CREAT);
     } else if (!strcmp(args[i], ">>&")) {
       if (newArgs == args) {
	 newArgs = subarray(args, 0, i);
       }
       redirect2(args[i+1], STDERR_FILENO, STDOUT_FILENO, O_RDWR | O_CREAT | O_APPEND);
     }
     // would be taking care of ;
     
     //would be taking care of cd?  
   }
   execute(args[0], newArgs);
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
	      printf("time to to execute all of those beautiful commands\n");
	      isolateRun(args);
	    } else if (!strcmp(args[i], "cd")) {  /* handles cd command  */
	      if (args[i+1] == NULL) {
		dprint("There is nothing after cd!\n");
		chdir(homeDirectory);
	      } else if (!strcmp(args[i+1], ";")) {
		dprint("There is a semicolon nothing after cd!\n");
		chdir(homeDirectory);
		args = &args[i+2];
		i = -1;
	      } else {
		printf("You want me to change the directory to '%s'?\n", args[i+1]);
		chdir(args[i+1]);
		if (args[i+2] != NULL && !strcmp(args[i+2], ";")) {
		  printf("I've just cahnged the directory to '%s' and now I might need to execute another command after '%s'\n",args[i+1],args[i+2]);
		  args = &args[i+3];
		  i = -1;
		}
	      }      	
	    } else if (!strcmp(args[i], ";")) {
	      dprint("we've encountered a juug semicolon\n");
	      isolateRun(subarray(args, 0, i));
	      args = &args[i+1];
	      i = -1;
	    }       // if args[i] == 'cd'	   
	  } //for i in args
	  isolateRun(args);
	  printf("There's a new player on the field\n");
	} // else from args[0] == NULl
    }
    return 0;
}
