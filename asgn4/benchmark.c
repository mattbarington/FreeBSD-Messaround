#include <stdio.h>
#include <stdlib.h>
//#include <time.h>
#include <string.h>
#include <sys/time.h>

//#define NUM_ARGS 2

//https://stackoverflow.com/questions/307596/time-difference-in-c
int diff_ms(struct timeval t1, struct timeval t2)
{
    return (((t1.tv_sec - t2.tv_sec) * 1000000) + 
            (t1.tv_usec - t2.tv_usec))/1000;
}

int main(int argc, char* argv[]) {

  double aofs_write_t;
  struct timeval aofs_write_s, aofs_write_f;
  double fbsd_write_t;
  struct timeval fbsd_write_s, fbsd_write_f;
  double aofs_create_t;
  struct timeval aofs_create_s, aofs_create_f;
  double fbsd_create_t;
  struct timeval fbsd_create_s, fbsd_create_f;
  double aofs_read_t;
  struct timeval aofs_read_s, aofs_read_f;
  double fbsd_read_t;
  struct timeval fbsd_read_s, fbsd_read_f;

  char run_aofs[] = "./init_fs ; ./run_fs aofs_test";

  char fbsd_folder[] = "fbsd_test";
  char aofs_folder[] = "aofs_test";
  char make_fbsd_folder[] = "mkdir fbsd_test";
  char make_aofs_folder[] = "mkdir aofs_test";
  char remove_fbsd_folder[] = "rm -rf fbsd_test";
  char remove_aofs_folder[] = "rm -rf aofs_test";

  char fbsd_touch_100_files[] = "touch fbsd_test/test_file";
  char aofs_touch_100_files[] = "touch aofs_test/test_file";
  char fbsd_write_file[] = "echo boi > fbsd_test/test_file1";
  char aofs_write_file[] = "echo boi > aofs_test/test_file1";
  char read_file[] = "cat test_file1";

  //check for correct number of arguments
  /*if(argc != NUM_ARGS) {
    printf("Incorrect number of arguments. Expected=%d. Actual=%d\n", NUM_ARGS, argc);
    return 1;
  }*/

  //initialize test
  system(make_fbsd_folder);
  system(make_aofs_folder);

  //run tests on FreeBSD file system
  int i;
  char touch_command[200];
  char numval[12];
  gettimeofday(&fbsd_create_s, NULL);
  for(i = 0; i < 100; ++i) {
    sprintf(numval, "%d", i);
    strcpy(touch_command, fbsd_touch_100_files);
    strcat(touch_command, numval);
    system(touch_command);
  }
  gettimeofday(&fbsd_create_f, NULL);
  printf("%d\n", diff_ms(fbsd_create_f, fbsd_create_s));
  printf("FreeBSD create 100: %lf\n", fbsd_create_t);
  
  system(remove_fbsd_folder);
  system(remove_aofs_folder);

  return 0;
}
