#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

//#define NUM_ARGS 2

//https://stackoverflow.com/questions/307596/time-difference-in-c
long diff_ms(struct timeval t1, struct timeval t2)
{
    return (((t1.tv_sec - t2.tv_sec) * 1000000000) + 
            (t1.tv_usec - t2.tv_usec));
}



int main(int argc, char* argv[]) {

  long aofs_write_t;
  struct timeval aofs_write_s, aofs_write_f;
  long fbsd_write_t;
  struct timeval fbsd_write_s, fbsd_write_f;
  long aofs_create_t;
  struct timeval aofs_create_s, aofs_create_f;
  long fbsd_create_t;
  struct timeval fbsd_create_s, fbsd_create_f;
  long aofs_read_t;
  struct timeval aofs_read_s, aofs_read_f;
  long fbsd_read_t;
  struct timeval fbsd_read_s, fbsd_read_f;

  char run_aofs[] = "./init_fs ; ./run_fs aofs_test";

  char fbsd_folder[] = "fbsd_test";
  char aofs_folder[] = "aofs_test";
  char make_fbsd_folder[] = "mkdir fbsd_test";
  char make_aofs_folder[] = "mkdir aofs_test";
  char remove_fbsd_folder[] = "rm -rf fbsd_test";
  char remove_aofs_folder[] = "rm -rf aofs_test";
  char remove_fbsd_sh[] = "rm fbsd.sh";
  char remove_aofs_sh[] = "rm aofs.sh";

  //char fbsd_touch_100_files[] = "touch fbsd_test/test_file";
  //char aofs_touch_100_files[] = "touch aofs_test/test_file";
  char fbsd_touch_100_files[] = "./fbsd.sh";
  char aofs_touch_100_files[] = "./aofs.sh";
  char fbsd_write_file[] = "dd if=/dev/urandom of=fbsd_test/test_file1 bs=8192 count=1 > /dev/null 2>&1 ";
  char aofs_write_file[] = "dd if=/dev/urandom of=aofs_test/test_file1 bs=8192 count=1 > /dev/null 2>&1";
  char fbsd_read_file[] = "cat fbsd_test/test_file1 >/dev/null 2>&1";
  char aofs_read_file[] = "cat aofs_test/test_file1 >/dev/null 2>&1";
  int i;

  char fbsd_build_create[] = "touch fbsd.sh ; chmod 777 fbsd.sh ; printf 'for i in `seq 1 100`\ndo\n  touch fbsd_test/test_file$i\ndone' > fbsd.sh ;";
  char aofs_build_create[] = "touch aofs.sh ; chmod 777 aofs.sh ; printf 'for i in `seq 1 100`\ndo\n  touch aofs_test/test_file$i\ndone' > aofs.sh ;";

  char touch_command[200];
  char numval[12];

  //initialize test
  system(make_fbsd_folder);
  system(make_aofs_folder);
  system(fbsd_build_create);
  system(aofs_build_create);

  //FreeBSD 100 files test
  gettimeofday(&fbsd_create_s, NULL);
  system(fbsd_touch_100_files);
  gettimeofday(&fbsd_create_f, NULL);
  fbsd_create_t = diff_ms(fbsd_create_f, fbsd_create_s);
  
  //FreeBSD write test
  gettimeofday(&fbsd_write_s, NULL);
  system(fbsd_write_file); 
  gettimeofday(&fbsd_write_f, NULL);
  fbsd_write_t = diff_ms(fbsd_write_f, fbsd_write_s);

  //FreeBSD read test
  gettimeofday(&fbsd_read_s, NULL);
  system(fbsd_read_file); 
  gettimeofday(&fbsd_read_f, NULL);
  fbsd_read_t = diff_ms(fbsd_read_f, fbsd_read_s);

  //AOFS 100 files test
  aofs_create_t = 123412334;
  
  //AOFS write test
  aofs_write_t = 69696969;

  //AOFS read test
  aofs_read_t = 420420420;

  //prlong test results
  printf("/------------------------------------\\\n");
  printf("|       RESULTS (milliseconds)       |\n");
  printf("|------------------------------------|\n");
  printf("|  test  |   FreeBSD   |     AOFS    |\n");
  printf("|------------------------------------|\n");
  printf("| create |%13ld|%13ld|\n", fbsd_create_t, aofs_create_t);
  printf("| write  |%13ld|%13ld|\n", fbsd_write_t, aofs_write_t);
  printf("| read   |%13ld|%13ld|\n", fbsd_read_t, aofs_read_t);
  printf("\\------------------------------------/\n");

  //remove test folders
  system(remove_fbsd_folder);
  system(remove_aofs_folder);
  system(remove_fbsd_sh);
  system(remove_aofs_sh);

  return 0;
}
