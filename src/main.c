#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMSZ     27

void sig_handler (int signal) {
  fprintf(stdout, "Signal: %d arrived\n", signal);
}

void child_kill_handler (int signal) {
  pid_t pid, new_pid;
  int status;
  pid = wait(&status);
  if (status == SIGTERM) {
    printf("Child process %d terminated with status %d... Restarting...\n", pid, status);
    //===== CREATE PROCESS AGAIN =====//
    new_pid = fork();
    printf("New sensor with pid: %d", new_pid);
    if (new_pid < 0) {
      perror("Fork error");
      exit(EXIT_FAILURE);
    }
    if (new_pid == 0) {
      char *args[3] = {"./bin/sensores", NULL};
      execvp(args[0], args);
    }
  } else if (status == SIGINT) {
    printf("Child process %d  terminated with status %d... Not restarting...\n", pid, status);
  }
}

int main(int argc, char *argv[]) {
  char sensor_l[25], sensor_r[25], sensor_c[25], reader_l[25], reader_r[25], reader_c[25];
  key_t key_T, key_W;
  if (signal(SIGTERM, sig_handler) == SIG_ERR) {
    printf("Can't handle SIGTERM\n");
  }
  if (signal(SIGCHLD, child_kill_handler) == SIG_ERR) {
    printf("Can't handle SIGCHLD\n");
  }
  //===== Get sensors locations =====//
  FILE *fp;
  fp = fopen("./config/main.config", "r");
  if (fp == NULL) {
    printf("No config file found!\n");
    exit(EXIT_FAILURE);
  }
  char buffer[25];
  if (fscanf(fp, "%s", buffer) != EOF) {
    strcpy(sensor_l, buffer);
  }
  if (fscanf(fp, "%s", buffer) != EOF ) {
    strcpy(sensor_c, buffer);
  }
  if (fscanf(fp, "%s", buffer) != EOF) {
    strcpy(sensor_r, buffer);
  }
  if (fscanf(fp, "%s", buffer) != EOF) {
    strcpy(reader_l, buffer);
  }
  if (fscanf(fp, "%s", buffer) != EOF) {
    strcpy(reader_c, buffer);
  }
  if (fscanf(fp, "%s", buffer) != EOF) {
    strcpy(reader_r, buffer);
  }
  fclose(fp);
  //===== GET PRINTER SHM ID =====//
  FILE *fpP;
  fpP = fopen("./config/printer.config", "r");
  if (fpP == NULL) {
    printf("No config file found!\n");
    exit(EXIT_FAILURE);
  }
  if (fscanf(fpP, "%s", buffer)) {
    key_T = atoi(buffer);
  }
  if(fscanf(fpP, "%s", buffer)) {
    key_W = atoi(buffer);
  }
  fclose(fpP);

  //===== SHARED MEMORY BLOCKS =====//
  // FREQUENCY
  int shmid_freq;
  char *shm_freq;
  key_t key_freq = 9999;
  if ((shmid_freq = shmget(key_freq, SHMSZ, IPC_CREAT | 0666)) < 0) {
    perror("shmget");
    return(1);
  }
  if ((shm_freq = shmat(shmid_freq, NULL, 0)) == (char *) -1) {
    perror("shmat");
    return(1);
  }
  // T
  int shmid_T;
  char *shm_T;
    if ((shmid_T = shmget(key_T, SHMSZ, IPC_CREAT | 0666)) < 0) {
    perror("shmget");
    return(1);
  }
  if ((shm_T = shmat(shmid_T, NULL, 0)) == (char *) -1) {
    perror("shmat");
    return(1);
  }
  // W
  int shmid_W;
  char *shm_W;
    if ((shmid_W = shmget(key_W, SHMSZ, IPC_CREAT | 0666)) < 0) {
    perror("shmget");
    return(1);
  }
  if ((shm_W = shmat(shmid_W, NULL, 0)) == (char *) -1) {
    perror("shmat");
    return(1);
  }

  //===== CREATE SENSORS =====//
  pid_t pid_l, pid_c, pid_r, pids[2];
  // Left sensor
  pid_l = fork();
  char *args_sensor_l[3] = {sensor_l, NULL};
  if (pid_l == -1) {
    perror("Fork failed");
    exit(EXIT_FAILURE);
  }
  if (pid_l == 0) { /* CHILD */
    execv(args_sensor_l[0], args_sensor_l);
    perror("Left sensor failed");
    exit(EXIT_FAILURE);
  } else { /* PARENT */
    pids[0] = pid_l;
  }
  // Center sensor
  pid_c = fork();
  char *args_sensor_c[3] = {sensor_c, NULL};
  if (pid_c == -1) {
    perror("Fork failed");
    exit(EXIT_FAILURE);
  }
  if (pid_c == 0) { /* CHILD */
    execv(args_sensor_c[0], args_sensor_c);
    perror("Center sensor failed");
    exit(EXIT_FAILURE);
  } else { /* PARENT */
    pids[1] = pid_c;
  }
  // Right sensor
  pid_r = fork();
  char *args_sensor_r[3] = {sensor_r, NULL};
  if (pid_r == -1) {
    perror("Fork failed");
    exit(EXIT_FAILURE);
  }
  if (pid_r == 0) { /* CHILD */
    execv(args_sensor_r[0], args_sensor_r);
    perror("Right sensor failed");
    exit(EXIT_FAILURE);
  } else { /* PARENT */
    pids[0] = pid_r;
  }
  
  //===== Create READERS =====//
  // LEFT READER
  pid_t readerL_pid;
  char *args_reader_l[3] = {reader_l, NULL};
  readerL_pid = fork();
  if (readerL_pid < 0) {
    perror("readerL fork failed");
    exit(EXIT_FAILURE);
  }
  if (readerL_pid == 0) {  //  lector_sensores
    execvp(args_reader_l[0], args_reader_l);
    // system("gnome-terminal -- ./bin/lectorL");
    perror("execvp lector_sensores failed");
    exit(EXIT_FAILURE);
  }
  // RIGHT READER
  pid_t readerR_pid;
  char *args_reader_r[3] = {reader_r, NULL};
  readerR_pid = fork();
  if (readerR_pid < 0) {
    perror("readerR fork failed");
    exit(EXIT_FAILURE);
  }
  if (readerR_pid == 0) {  //  lector_sensores
    execvp(args_reader_r[0], args_reader_r);
    // system("gnome-terminal -- ./bin/lectorR");
    perror("execvp lector_sensores failed");
    exit(EXIT_FAILURE);
  }
  // CENTER READER
  pid_t readerC_pid;
  char *args_reader_c[3] = {reader_c, NULL};
  readerC_pid = fork();
  if (readerC_pid < 0) {
    perror("center reader fork failed");
    exit(EXIT_FAILURE);
  }
  if (readerC_pid == 0) {  //  lector_sensores
    execvp(args_reader_c[0], args_reader_c);
    // system("gnome-terminal -- ./bin/lectorC");
    perror("execvp lector_sensores failed");
    exit(EXIT_FAILURE);
  }
  
  //===== CREATE PRINTER =====//
  pid_t printer_pid;
  printf("CREATING PRINTER\n");
  printer_pid = fork();
  if (printer_pid < 0) {
    perror("printer fork failed");
    exit(EXIT_FAILURE);
  }
  if (printer_pid == 0) {  //  printer
    system("gnome-terminal -- ./bin/printer");
    exit(EXIT_FAILURE);
  }
  //===== Show ids of child processes =====//
  /* for (i = 0; i < 6; i = i + 1) {
    fprintf(stdout, "Sensor #%d pid: %d\n", (i + 1), pids[i]);
  }
  fprintf(stdout, "Lector pid: %d\n", lector_pid);
  // fprintf(stdout, "Printer pid: %d\n", printer_pid);
  //===== Wait =====//
  // for (i = 0; i < numProcesses; i = i + 1) {
    // w = waitpid(pids[i], &status);
    // fprintf(stdout,"%d\n", w);
  // }*/
  
  //===== MENU =====//
  sleep(2);
  int opcion;
  int freq;
  float T, W;
  while (1) {
    printf("\nBienvenido al administrador del programa, seleccione una de las opciones a ejecutar:\n");
    printf("1) Modificar la frecuencia de los sensores.\n");
    printf("2) Modificar el valor de T.\n");
    printf("3) Modificar el valor de W.\n");
    scanf("%d", &opcion);
    if (opcion < 1 || opcion > 6) {
      printf("\nOpcion no dentro del rango, escoja nuevamente.\n");
    } else {
      switch(opcion) {
        case 1:
	  printf("Ha seleccionado la opción de modificar la frecuencia de los sensores.\nIngrese la nueva frecuencia: ");
          scanf("%d", &freq);
	  sprintf(shm_freq, "%d", freq);
	  break;
	case 2:
	  printf("Ingrese el nuevo valor de T: ");
	  scanf("%f", &T);
	  sprintf(shm_T, "%f", T);
	  break;
	case 3:
          printf("Ingrese el nuevo valor de W: ");
          scanf("%f", &W);
          sprintf(shm_W, "%f", W);
          break;

      }
    }

  }
  getchar();
  return (0);
}
