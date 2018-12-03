#include <sys/types.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#define SHMSZ 27

/* struct params {
  float distance;
  float angle;
}; */


void *aux_func(void *param);

char *shm_p;

int main () {
  key_t key_d, key_t, key_p;
  int shmid_d, shmid_t, shmid_p;
  char *shm_d, *shm_t;
  int FLAGS = 0666;
  //===== To store the values from the shared memory segment =====//
  char tmp_t[SHMSZ];
  char tmp_d[SHMSZ];
  char old_t[SHMSZ];
  char old_d[SHMSZ];
  //===== To indicate when a pair of distance-angle has been obtained=====//
  int switch_t = 0, switch_d = 0;
  //===== pthreads =====//
  pthread_t tid;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  //===== DISTANCE SENSOR =====//
  key_d = 1234;
  shmid_d = shmget(key_d, SHMSZ, FLAGS);
  if ( shmid_d < 0) {
    perror("smget");
    return (1);
  }
  shm_d = shmat(shmid_d, NULL, 0);
  if (shm_d == (char *) -1) {
    perror("shmat");
    return (1);
  }
  //===== GIROSCOPE SENSOR =====//
  key_t = 5678; 
  shmid_t = shmget(key_t, SHMSZ, FLAGS);
  if (shmid_t < 0) {
    perror("shmget");
    return (1);  
  }
  shm_t = shmat(shmid_t, NULL, 0);
  if (shm_t == (char *) -1) {
    perror("shmat");
    return (1);
  }
  //===== PRINTER PROCESS SHARED MEMORY SEGMENT =====//
  key_p = 2795;
  shmid_p = shmget(key_p, SHMSZ, IPC_CREAT | 0666);
  if (shmid_p < 0) {
    perror("shmget");
    return (1);
  }
  shm_p = shmat(shmid_p, NULL, 0);
  if (shm_p == (char *)-1) {
    perror("shmat");
    return (1);
  }
  
  while (1) {
    strcpy(tmp_t, shm_t);
    if ((strcmp(tmp_t, "--") != 0) && (strcmp(old_t, tmp_t) != 0) && (switch_t == 0)) {
      switch_t = 1;
      // fprintf(stdout, "giroscopio: %s\n", tmp_t);
      strcpy(old_t, tmp_t);
    }
    if ((strcmp(tmp_d, shm_d) != 0) && (switch_d == 0)) {
      switch_d = 1;
      // fprintf(stderr, "distancia: %s\n", tmp_d);
      strcpy(old_d, shm_d);
    }
    strcpy(tmp_d, shm_d);
    if (switch_d == 1 && switch_t == 1) {
      switch_t = 0;
      switch_d = 0;
      // fprintf(stdout, "Se han obtenido los valores de distancia y giroscopio para hacer el calculo\n");
      // fprintf(stdout, "Los valores son: %f - %f\n", strtod(old_t, NULL), strtod(old_d, NULL));
      float *numbers = malloc(2 * sizeof(float));
      numbers[0] = strtod(old_t, NULL);
      numbers[1] = strtod(old_d, NULL);
      pthread_create(&tid, &attr, aux_func, (void *)numbers);
    }
  }
  return(0);
}

void *aux_func (void *param) {
  float *numbers = (float *)param;
  // fprintf(stdout, "Values from thread are: %f - %f\n", numbers[0], numbers[1]);
  // sleep(3);
  // float sum = numbers[0] + numbers[1];
  float real_distance = numbers[1] * cos(numbers[0]);
  // fprintf(stdout, "Values are: %f & %f, and the sum is: %f\n", numbers[0], numbers[1], real_distance);
  // shm_p = (char *)sum;
  sprintf(shm_p,"%f", real_distance);
  
  // printf("Ending thread\n");
  pthread_exit(0);
}
