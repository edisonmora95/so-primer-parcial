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

float T = 0.2;
float W = 2;

void *car_obstacle(void *param);

int main() {
  printf("PRINTER PROCESS\n");
  struct distance distance;
  int shmid_l, shmid_r, shmid_c, shmid_T, shmid_W; // ID of the shared memory segment
  key_t key_r, key_l, key_c, key_T, key_W; // Key of the shared memory
  key_l = 1002;
  key_r = 2002;
  key_c = 3002;
  int FLAGS = 0666;
  char *shm_l, *shm_r, *shm_c, *shm_T, *shm_W;
  char tmp_l[SHMSZ], tmp_r[SHMSZ], tmp_c[SHMSZ], tmp_T[SHMSZ], tmp_W[SHMSZ];
  int switch_l, switch_r, switch_c;

  //===== GET VALUES FROM CONFIG =====//
  FILE *fp;
  fp = fopen("./config/printer.config", "r");
  if (fp == NULL) {
    printf("No config file found!\n");
    exit(EXIT_FAILURE);
  }
  char buffer[25];
  if (fscanf(fp, "%s", buffer)) {
    key_T = atoi(buffer);
  }
  if (fscanf(fp, "%s", buffer)) {
    key_W = atoi(buffer);
  }
  fclose(fp);
  
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
  //===== MEM BLOCK FOR T VALUE =====//
  shmid_T = shmget(key_T, SHMSZ, IPC_CREAT | 0666);
  if (shmid_T < 0) {
    perror("Error creating shared memory segment");
    exit(1);
  }
  shm_T = shmat(shmid_T, NULL, 0);
  if (shm_T == (char *)-1) {
    perror("Error attaching the shared memory segment");
    exit(1);
  }
  //===== MEM BLOCK FOR W VALUE =====//
  shmid_W = shmget(key_W, SHMSZ, IPC_CREAT | 0666);
  if (shmid_W < 0) {
    perror("Error creating shared memory segment");
    exit(1);
  }
  shm_W = shmat(shmid_W, NULL, 0);
  if (shm_W == (char *)-1) {
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
    if (strcmp(tmp_T, shm_T) != 0) {
      // fprintf(stdout, "New value of T: %f\n", atof(shm_T));
      strcpy(tmp_T, shm_T);
      T = atof(shm_T);
    }
    if (strcmp(tmp_W, shm_W) != 0) {
      // fprintf(stdout, "New value of W: %f\n", atof(shm_W));
      strcpy(tmp_W, shm_W);
      W = atof(shm_W);
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
  getchar();
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
