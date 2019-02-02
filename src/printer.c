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
#include <unistd.h>

#define SHMSZ 27


int main() {
  printf("PRINTER PROCESS\n");
  int shmid_l, shmid_r, shmid_c; // ID of the shared memory segment
  key_t key_r, key_l, key_c; // Key of the shared memory
  key_l = 1002;
  key_r = 2002;
  key_c = 3002;
  int FLAGS = 0666;
  char *shm_l, *shm_r, *shm_c;
  char tmp_l[SHMSZ], tmp_r[SHMSZ], tmp_c[SHMSZ];
  int switch_l, switch_r, switch_c;
  //===== MEM BLOCK FOR LEFT READER =====//
  shmid_l = shmget(key_l, SHMSZ, IPC_CREAT | 0666);
  if (shmid_l < 0) {
    perror("Error creating shared memory segment");
    exit(1);
  }
  shm_l = shmat(shmid_l, NULL, 0);
  if (shm_l == (char *)-1) {
    perror("Error attaching the shared memory segment");
    exit(1);
  }
  //===== MEM BLOCK FOR RIGHT READER =====//
  shmid_r = shmget(key_r, SHMSZ, IPC_CREAT | 0666);
  if (shmid_r < 0) {
    perror("Error creating shared memory segment");
    exit(1);
  }
  shm_r = shmat(shmid_r, NULL, 0);
  if (shm_l == (char *)-1) {
    perror("Error attaching the shared memory segment");
    exit(1);
  }
  //===== MEM BLOCK FOR CENTER READER =====//
  shmid_c = shmget(key_c, SHMSZ, IPC_CREAT | 0666);
  if (shmid_c < 0) {
    perror("Error creating shared memory segment");
    exit(1);
  }
  shm_c = shmat(shmid_c, NULL, 0);
  if (shm_c == (char *)-1) {
    perror("Error attaching the shared memory segment");
    exit(1);
  }
  while (1) {
    if (strcmp(tmp_l, shm_l) != 0) {
      fprintf(stdout, "Distance from left reader is: %s\n", shm_l);
      switch_l = 1;
      strcpy(tmp_l, shm_l);
    }
    if (strcmp(tmp_r, shm_r) != 0) {
      fprintf(stdout, "Distance from right reader is: %s\n", shm_r);
      switch_r = 1;
      strcpy(tmp_r, shm_r);
    }
    if (strcmp(tmp_c, shm_c) != 0) {
      fprintf(stdout, "Distance from center reader is: %s\n", shm_c);
      switch_c = 1;
      strcpy(tmp_c, shm_c);
    }
    if ((switch_l == 1) && (switch_r == 1) && (switch_c == 1)) {
      fprintf(stdout, "Y AHOOORAAAAA\n");
      switch_l = 0;
      switch_r = 0;
      switch_c = 0;
    }
  }

  return(0);
}
