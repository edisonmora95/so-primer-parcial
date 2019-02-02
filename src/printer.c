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
#include <pthread.h>
#include <math.h>
#define SHMSZ 27

pthread_mutex_t mutex;

struct distance {
  double left;
  double center;
  double right;
};

double T = 0.5;
double W = 2;

void *car_obstacle(void *param);

int main() {
  printf("PRINTER PROCESS\n");
  struct distance distance;
  int shmid_l, shmid_r, shmid_c; // ID of the shared memory segment
  key_t key_r, key_l, key_c; // Key of the shared memory
  key_l = 1002;
  key_r = 2002;
  key_c = 3002;
  int FLAGS = 0666;
  char *shm_l, *shm_r, *shm_c;
  char tmp_l[SHMSZ], tmp_r[SHMSZ], tmp_c[SHMSZ];
  int switch_l, switch_r, switch_c;

  pthread_t tid;
  pthread_attr_t attr;
  pthread_attr_init(&attr);

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
      switch_l = 0;
      switch_r = 0;
      switch_c = 0;
      distance.left = atoi(tmp_l);
      distance.center = atoi(tmp_c);
      distance.right = atoi(tmp_r);
      pthread_create(&tid, &attr, car_obstacle, (void *)&distance);

    }
  }
  sleep(5);
  return(0);
}

void *car_obstacle (void *param) {
  struct distance *distance = param;
  float sum = 0.0, sd = 0.0, mean;
  
  sum = distance->left + distance->center + distance->right;
  mean = sum/3;
  sd = pow(distance->left - mean, 2) + pow(distance->center - mean, 2) + pow(distance->right - mean, 2);
  sd = sqrt(sd/3);
  // printf("Left: %f\n", distance->left);
  // printf("Center: %f\n", distance->center);
  // printf("Right: %f\n", distance->right);
  printf("SD: %f\n", sd);

  if ((fabs(distance->left - distance->center) < (T*sd)) && (fabs(distance->right - distance->center) < (T*sd))) {
    printf("ES UN CARRO\n");
  }

  if ((fabs(distance->left - distance->center) == (W*sd)) || (fabs(distance->right - distance->center) == (W*sd)) || (fabs(distance->left - distance->right) == (W*sd))) {
    printf("ES UN OBSTACULO\n");
  }

  pthread_exit(0);
}
