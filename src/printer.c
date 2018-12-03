/*
 * This process will read from a shared memory
 * When it finds a new value, it will write it to the console
 * */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHMSZ 27


int main() {
  int shmid; // ID of the shared memory segment
  key_t key = 2795; // Key of the shared memory
  int FLAGS = 0666;
  char *shm;
  char new_tmp[SHMSZ];

  /*
   * First get the segment of shared memory
   * */
  printf("Creating shared memory...\n");
  shmid = shmget(key, SHMSZ, FLAGS);
  if (shmid < 0) {
    perror("Error creating shared memory segment");
    exit(1);
  }
  printf("Attaching shared memory segment...\n");
  /*
   * Then we attach the segment
   * */
  shm = shmat(shmid, NULL, 0);
  if (shm == (char *)-1) {
    perror("Error attaching the shared memory segment");
    exit(1);
  }
  /*
   * Then we open the file to write the results to*
   * */
  FILE *fp;
  fp = fopen("./pruebas.csv", "a");
  /*
   * Now we read from the shared memory segment until we get a value
   * */
  while (1) {
    if (strcmp(new_tmp, shm) != 0) {
      fprintf(stdout, "Distance is: %s\n", new_tmp);
      // fprintf(fp, "%s\n", new_tmp);
    }
    strcpy(new_tmp, shm);
    // if ()
    // fprintf(stdout, "shared memory: %s \n", new_tmp);
  }
  return(0);
}
