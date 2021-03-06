/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Daniel Ochoa Donoso 2010 <dochoa@fiec.espol.edu.ec>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#define SHMSZ     27
#define MAX_SAMPLES 100
#define MAX_SAMPLES_THETA 50
#define DIST 10
#define PI 3.14159265
        /* ranf() is uniform in 0..1 */

float box_muller(float m, float s);	/* normal random variate generator */
void *read_memory(void *param);

int freq = 0; // Frequency to write the data to the shared memory block

int main() {
  char c;
  int shmidd,shmidt;
  key_t keyd, keyt, key_freq;
  char *shmd, *shmt;
  int i,j;
  float distances[MAX_SAMPLES];
  float angles[MAX_SAMPLES_THETA];
  float anglesD[MAX_SAMPLES];
  float mu,sigma,delta_theta;

  struct timespec tim, tim2;
  tim.tv_sec = 1;
  tim.tv_nsec = 0;
  printf("Left sensor created\n");
  //===== READ PARAMETERS FROM CONFIG FILE =====//
  FILE *fp;
  fp = fopen("./config/sensorL.config", "r");
  if (fp == NULL) {
    printf("No se abrir un archivo\n");
    return(0);
  }
  char str[5];
  // First value is the frequency
  if (fscanf(fp, "%s", str) != EOF) {
    keyd = atoi(str);
  }
  // Second value is the shared memory block
  if (fscanf(fp, "%s", str) != EOF) {
    freq = atoi(str);
  }
  // Third value is the shared memory block for the frequency sync
  if (fscanf(fp, "%s", str) != EOF) {
    key_freq = atoi(str);
  }
  fclose(fp);
  
  if ((shmidd = shmget(keyd, SHMSZ, IPC_CREAT | 0666)) < 0) {
    perror("shmget");
    return(1);
  }    
  if ((shmd = shmat(shmidd, NULL, 0)) == (char *) -1) {
    perror("shmat");
    return(1);
  }    
  keyt = keyd + 1;
  if ((shmidt = shmget(keyt, SHMSZ, IPC_CREAT | 0666)) < 0) {
    perror("shmget");
    return(1);
  }    
  if ((shmt = shmat(shmidt, NULL, 0)) == (char *) -1) {
    perror("shmat");
    return(1);
  }

  // Create thread to read shared memory block
  pthread_t tid_freq;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create(&tid_freq, &attr, read_memory, (void *) key_freq);
  
  mu = 0;
  sigma = 25;    
  // sprintf(shmd, "%s", "---");
  // sprintf(shmt, "%s", "---");
  sleep(2);
  for (i = 0; i < MAX_SAMPLES_THETA; i++) {
    angles[i]=box_muller(mu,sigma);
  }
	
  j = -1;
  for (i = 0; i < MAX_SAMPLES_THETA - 1; i++) {        
    if (j++ < MAX_SAMPLES) anglesD[j] = angles[i];
    if (j++ < MAX_SAMPLES) {
      delta_theta=abs(angles[i+1] - angles[i]) / 2;
      if (angles[i+1] > angles[i]) 
        anglesD[j] = angles[i] + delta_theta;
      else
        anglesD[j] = angles[i] - delta_theta;
    }
  }
   
  for (i = 0; i < j; i++) {
    distances[i] = DIST / cos(anglesD[i]/180*PI);		
  }
  for (i = 0; i < j; i++) {
    if (nanosleep(&tim , &tim2) < 0 ) {
      printf("Nano sleep failed \n");
      return -1;
    }
    //printf("Distance: %f\n", distances[i]);
    sprintf(shmd,"%f",distances[i]);
    if (i%freq == 0) {
      // printf("Angle: %f\n", anglesD[i]);
      sprintf(shmt,"%f",anglesD[i]); 
    } else {
      strcpy(shmt,"--");
    }
  }
  sprintf(shmd, "%s", "DONE");
  sprintf(shmt, "%s", "DONE");
  pthread_join(tid_freq, NULL);
  return(0);
}

float box_muller(float m, float s) {	/* normal random variate generator */
				        /* mean m, standard deviation s */
  float x1, x2, w, y1;
  static float y2;
  static int use_last = 0;
  if (use_last) {		        /* use value from previous call */
    y1 = y2;
    use_last = 0;
  } else {
    do {
      x1 = 2.0 * ((double)(rand())/RAND_MAX)- 1.0;
      x2 = 2.0 * ((double)(rand())/RAND_MAX) - 1.0;
      w = x1 * x1 + x2 * x2;
    } while ( w >= 1.0 );

    w = sqrt( (-2.0 * log( w ) ) / w );
    y1 = x1 * w;
    y2 = x2 * w;
    use_last = 1;
  }
  return( m + y1 * s );
}

void *read_memory (void *param) {
  key_t key_freq = (key_t)param;
  char tmp_freq[SHMSZ];
  int shmid_freq;
  char *shm_freq;
  shmid_freq = shmget(key_freq, SHMSZ, 0666);
  if (shmid_freq < 0) {
    perror("shmget");
    pthread_exit(0);
  }
  shm_freq = shmat(shmid_freq, NULL, 0);
  if (shm_freq == (char *) -1) {
    printf("Error\n");
    perror("shmat");
    pthread_exit(0);
  }
  strcpy(tmp_freq, shm_freq);
  while (1) {
   if (strcmp(tmp_freq, shm_freq) != 0 && strcmp(shm_freq, "") != 0) {
     strcpy(tmp_freq, shm_freq);
     freq = atoi(shm_freq);
   }
  }
  pthread_exit(0);
}
