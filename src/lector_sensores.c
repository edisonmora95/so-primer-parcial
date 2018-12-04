#include <sys/types.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SHMSZ 27
#define PI 3.14159265

void *aux_func(void *param);

char *shm_p;
pthread_mutex_t mutex;
int contador = 0;

char old_t[SHMSZ];
char old_d[SHMSZ];

//===== Time measurement =====//
clock_t start, end;
double time_used;
double avg_time;

int numThreads = 0;

int main () {
  key_t key_d, key_t, key_p;
  int shmid_d, shmid_t, shmid_p;
  char *shm_d, *shm_t;
  int FLAGS = 0666;
  //===== To store the values from the shared memory segment =====//
  char tmp_t[SHMSZ];
  char tmp_d[SHMSZ];
  // char old_t[SHMSZ];
  // char old_d[SHMSZ];
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
  /* key_p = 2795;
  shmid_p = shmget(key_p, SHMSZ, IPC_CREAT | 0666);
  if (shmid_p < 0) {
    perror("shmget");
    return (1);
  }
  shm_p = shmat(shmid_p, NULL, 0);
  if (shm_p == (char *)-1) {
    perror("shmat");
    return (1);
  } */
  
  while (1) {
    strcpy(tmp_t, shm_t);
    if ((strcmp(tmp_t, "DONE") == 0) && (strcmp(tmp_d, "DONE") == 0)) {
      break;
    } 
    if ((strcmp(tmp_t, "--") != 0) && (strcmp(old_t, tmp_t) != 0) && (switch_t == 0)) {
      start = clock();
      switch_t = 1;
      strcpy(old_t, tmp_t);
    }
    if ((strcmp(tmp_d, shm_d) != 0) && (strcmp(old_d, tmp_d) != 0)  && (switch_d == 0)) {
      switch_d = 1;
      strcpy(old_d, shm_d);
    }
    strcpy(tmp_d, shm_d);
    if (switch_d == 1 && switch_t == 1) {
      switch_t = 0;
      switch_d = 0;
      numThreads = numThreads + 1;
      /* float *numbers = malloc(2 * sizeof(float));
      numbers[0] = strtod(old_t, NULL);
      numbers[1] = strtod(old_d, NULL);
      pthread_create(&tid, &attr, aux_func, (void *)numbers); */
      pthread_create(&tid, &attr, aux_func, NULL);
    }
  }
  while (numThreads > 0) {
    sleep(1);
  }
  sprintf(shm_t, "--");
  sprintf(shm_d, "--");
  printf("Average time: %f", avg_time / contador);
  getchar();
  return(0);
}

void *aux_func (void *param) {
  pthread_mutex_lock(&mutex);
  contador = contador + 1;
  // float *numbers = (float *)param;
  // float real_distance = numbers[1] * cos(numbers[0] / (180 * PI));
  float distance = strtod(old_d, NULL);
  float angle = strtod(old_t, NULL);
  float real_distance = distance * cos(angle / (180 * PI));
  // fprintf(stdout, "%i) Values are: %f & %f, and the real distance is: %f\n", contador, numbers[0], numbers[1], real_distance);
  end = clock();
  time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  fprintf(stdout, "%i) Values are: %f & %f, and the real distance is: %f | time: %f\n", contador, distance, angle, real_distance, time_used);
  avg_time = avg_time + time_used;
  numThreads --;
  // sprintf(shm_p,"%f", real_distance);
  pthread_mutex_unlock(&mutex);
  pthread_exit(0);
}
