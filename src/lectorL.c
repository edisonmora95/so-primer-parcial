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

//===== FUNCTION DECLARATIONS =====//
void *aux_func(void *param);
void *read_giroscope(void *param);
void *read_distance(void *param);
void print_distance();

char *shm_p;
pthread_mutex_t mutex;
int contador = 0;

//===== Time measurement =====//
clock_t start, end;
double time_used;
double avg_time;

int numThreads = 0;

//===== SWITCHES =====//
int switch_d = 0; // Distance switch
int switch_g = 0; // Giroscope switch

//===== TEMP VARIABLES FOR SHM =====//
char tmp_g[SHMSZ];
char tmp_d[SHMSZ];

//===== SHARED MEMORY VARIABLES =====//
char *shm_d, *shm_g;

int main () {
  key_t key_d, key_g;
  //===== READ PARAMETERS FROM CONFIG FILE =====//
  FILE *fp;
  char str[5];
  fp = fopen("./config/sensorL.config", "r");
  if (fp == NULL) {
    printf("No se abrir un archivo\n");
    return(0);
  }
  if (fscanf(fp, "%s", str) != EOF) {
    key_d = atoi(str);
    key_g = key_d + 1;
  }
  fclose(fp);

  int shmid_d, shmid_g;
  int FLAGS = 0666;
  //===== pthreads =====//
  pthread_t tid;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  
  //===== DISTANCE SENSOR =====//
  shmid_d = shmget(key_d, SHMSZ, FLAGS);
  if (shmid_d < 0) {
    perror("smget");
    return (1);
  }
  shm_d = shmat(shmid_d, NULL, 0);
  if (shm_d == (char *) -1) {
    perror("shmat");
    return (1);
  }
  //===== GIROSCOPE SENSOR =====//
  shmid_g = shmget(key_g, SHMSZ, FLAGS);
  if (shmid_g < 0) {
    perror("shmget");
    return (1);  
  }
  shm_g = shmat(shmid_g, NULL, 0);
  if (shm_g == (char *) -1) {
    perror("shmat");
    return (1);
  }

  //===== CREATE THREADS =====//
  pthread_t tid_g; // Giroscope reader thread
  pthread_create(&tid_g, &attr, read_giroscope, NULL);
  pthread_t tid_d; // Distance sensor reader thread
  pthread_create(&tid_d, &attr, read_distance, NULL);

  pthread_join(tid_d, NULL);
  pthread_join(tid_g, NULL);
  
  sprintf(shm_g, "--");
  sprintf(shm_d, "--");
  sprintf(tmp_d, "--");
  sprintf(tmp_g, "--");
  printf("Average time: %f", avg_time / contador);
  getchar();
  return(0);
}
// Thread for reading gyroscope
void *read_giroscope (void *param) {
  strcpy(tmp_g, shm_g);
  while (1) {
    // When both switches are 'on', it means a pair of distance-angle has been read
    if (switch_d == 1 && switch_g == 1) {
      print_distance();
    }
    // When a new valid value is read, it sets the gyroscope switch to 'on'
    if (strcmp(tmp_g, shm_g) != 0 && strcmp("--", shm_g) != 0 && (switch_g == 0)) {
      printf("Nuevo valor de giroscopio leido: ");
      printf("%s\n", shm_g);
      strcpy(tmp_g, shm_g);
      switch_g = 1;
    }
    if (strcmp(tmp_g, "DONE") == 0) {
      break;
    }
  }
  pthread_exit(0);
}
// Thread for reading distnce sensor
void *read_distance (void *param) {
  strcpy(tmp_d, shm_d);
  while (1) {
    /*
    if (switch_d == 1 && switch_g == 1) {
      // print_distance();
    } */
    if ((strcmp(tmp_d, "--") != 0) && (switch_d == 0) && (strcmp(tmp_d, shm_d) != 0)) {
      printf("Valor de distancia leido: ");
      printf("%s\n", shm_d);
      switch_d = 1;
    }
    if (strcmp(tmp_d, "DONE") == 0) {
      break;
    }
    strcpy(tmp_d, shm_d);
  }
  pthread_exit(0);
}

void print_distance () {
  float distance = strtod(tmp_d, NULL);
  float angle = strtod(tmp_g, NULL);
  float real_distance = distance * cos(angle / (180 * PI));
  fprintf(stdout, "%i) Values are : %f & %f, real distance is: %f\n", contador, distance, angle, real_distance);
  strcpy(tmp_g, "--");
  strcpy(tmp_d, "--");
  switch_d = 0;
  switch_g = 0;
  contador++;
}


