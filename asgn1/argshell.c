#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

extern char ** get_args();

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
	  printf("doing the process thing\n");
	  int pid = fork();
	  if (pid) { //pid != 0
	    printf("am parent\n");
	    waitpid(pid,NULL, 0);	   
	  } else {
	    printf("am child\n");
	    int s = execvp(args[0], &args[1]);
	    printf("status : %d\n", s);
	  }
	}
    }
}
