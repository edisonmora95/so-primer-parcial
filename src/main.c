#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

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
  char sensor_l[25], sensor_r[25], sensor_c[25], lector_sensores[25];
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
    strcpy(lector_sensores, buffer);
  }
  // printf("SensorL: %s\n", sensor_l);
  // printf("SensorR: %s\n", sensor_r);
  // printf("SensorC: %s\n", sensor_c);
  fclose(fp);
  //===== Create sensors =====//
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
  //===== Create lector_sensores process =====//
  pid_t lectorL_pid;
  // char *args_lector[3] = {lector_sensores, NULL};
  lectorL_pid = fork();
  if (lectorL_pid < 0) {
    perror("lector_sensores fork failed");
    exit(EXIT_FAILURE);
  }
  if (lectorL_pid == 0) {  //  lector_sensores
    // execvp(args_lector[0], args_lector);
    system("gnome-terminal -- ./bin/lectorL");
    // perror("execvp lector_sensores failed");
    exit(EXIT_FAILURE);
  }
  //===== Create lector_sensores process =====//
  pid_t lectorR_pid;
  // char *args_lector[3] = {lector_sensores, NULL};
  lectorR_pid = fork();
  if (lectorR_pid < 0) {
    perror("lector_sensores fork failed");
    exit(EXIT_FAILURE);
  }
  if (lectorR_pid == 0) {  //  lector_sensores
    // execvp(args_lector[0], args_lector);
    system("gnome-terminal -- ./bin/lectorR");
    // perror("execvp lector_sensores failed");
    exit(EXIT_FAILURE);
  }
  //===== Create lector_sensores process =====//
  pid_t lectorC_pid;
  // char *args_lector[3] = {lector_sensores, NULL};
  lectorC_pid = fork();
  if (lectorC_pid < 0) {
    perror("lector_sensores fork failed");
    exit(EXIT_FAILURE);
  }
  if (lectorC_pid == 0) {  //  lector_sensores
    // execvp(args_lector[0], args_lector);
    system("gnome-terminal -- ./bin/lectorC");
    // perror("execvp lector_sensores failed");
    exit(EXIT_FAILURE);
  }

  /*//===== Show ids of child processes =====//
  for (i = 0; i < numProcesses; i = i + 1) {
    fprintf(stdout, "Sensor #%d pid: %d\n", (i + 1), pids[i]);
  }
  fprintf(stdout, "Lector pid: %d\n", lector_pid);
  // fprintf(stdout, "Printer pid: %d\n", printer_pid);
  //===== Wait =====//
  // for (i = 0; i < numProcesses; i = i + 1) {
    // w = waitpid(pids[i], &status);
    // fprintf(stdout,"%d\n", w);
  // }*/
  getchar();
  return (0);
}
