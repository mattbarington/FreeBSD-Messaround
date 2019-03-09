#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//#define NUM_ARGS 2

int main(int argc, char* argv[]) {

  double aofs_write_t;
  time_t aofs_write_s, aofs_write_f;
  double fbsd_write_t;
  time_t fbsd_write_s, fbsd_write_f;
  double aofs_create_t;
  time_t aofs_create_s, aofs_create_f;
  double fbsd_create_t;
  time_t fbsd_create_s, fbsd_create_f;
  double aofs_read_t;
  time_t aofs_read_s, aofs_read_f;
  double fbsd_read_t;
  time_t fbsd_read_s, fbsd_read_f;

  char run_aofs[] = "./init_fs ; ./run_fs aofs_test";

  char fbsd_folder[] = "fbsd_test";
  char aofs_folder[] = "aofs_test";
  char goto_fbsd_folder[] = "cd fbsd_test";
  char goto_aofs_folder[] = "cd ../aofs_test";
  char make_fbsd_folder[] = "mkdir fbsd_test";
  char make_aofs_folder[] = "mkdir aofs_test";

  char touch_100_files[] = "touch test_file";
  char write_file[] = "echo boi > test_file1";
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
  system(goto_fbsd_folder);
  int i;
  char touch_command[200];
  char numval[12];
  fbsd_create_s = time(NULL);
  for(i = 0; i < 100; ++i) {
    itoa(i, numval, 10);
    strcpy(touch_command, touch_100_files);
    strcat(touch_command, numval);
    system(touch_command);
  }
  fbsd_create_f = time(NULL);
  fbsd_create_t = difftime(fbsd_create_f, fbsd_create_s);
  printf("FreeBSD create 100: %lf\n", fbsd_create_t);
  
  return 0;
}
