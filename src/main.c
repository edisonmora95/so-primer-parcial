#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

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
  if (signal(SIGTERM, sig_handler) == SIG_ERR) {
    printf("Can't handle SIGTERM\n");
  }
  if (signal(SIGCHLD, child_kill_handler) == SIG_ERR) {
    printf("Can't handle SIGCHLD\n");
  }
  //===== Get the number of processes to be created at first =====//
  if (argc < 2 ) {
    printf("Please indicate the number of sensors to be created\n");
    exit(EXIT_FAILURE);
  }
  if (argc > 2) {
    printf("Too many arguments\n");
    exit(EXIT_FAILURE);
  }
  int numProcesses = 0;
  sscanf(argv[1], "%d", &numProcesses);
  if (numProcesses > 10) {
    printf("Too many processes");
    exit(EXIT_FAILURE);
  }
  //===== Create by using fork and execvp =====//
  pid_t cpid, w, pids[numProcesses];
  char *args[3] = {"./bin/sensores", NULL};
  int i, status;
  for (i = 0; i < numProcesses; i = i + 1) {
    cpid = fork();
    if (cpid == -1) {
      perror("Fork failed");
      exit(EXIT_FAILURE);
    }
    if (cpid == 0) {  /* Child */
      execvp(args[0], args);
      perror("execvp failed\n");
    } else {  /* Parent */
      pids[i] = cpid;
    }
  }
  //===== Create lector_sensores process =====//
  pid_t lector_pid;
  char *args_lector[3] = {"./bin/lector_sensores", NULL};
  lector_pid = fork();
  if (lector_pid < 0) {
    perror("lector_sensores fork failed");
    exit(EXIT_FAILURE);
  }
  if (lector_pid == 0) {  /* lector_sensores */
    execvp(args_lector[0], args_lector);
    // system("gnome-terminal -- ./bin/lector_sensores");
    perror("execvp lector_sensores failed");
    exit(EXIT_FAILURE);
  }
  //===== Create printer process =====//
  pid_t printer_pid;
  char *args_printer[3] = {"./bin/printer", NULL};
  printer_pid = fork();
  if (printer_pid < 0) {
    perror("printer fork failed");
    exit(EXIT_FAILURE);
  }
  if (printer_pid == 0) {
    // execvp(args_printer[0], args_printer);
    system("gnome-terminal -- ./bin/printer");
    // perror("printer execvp failed");
    exit(EXIT_FAILURE);
  }
  //===== Show ids of child processes =====//
  for (i = 0; i < numProcesses; i = i + 1) {
    fprintf(stdout, "Sensor #%d pid: %d\n", (i + 1), pids[i]);
  }
  fprintf(stdout, "Lector pid: %d\n", lector_pid);
  fprintf(stdout, "Printer pid: %d\n", printer_pid);
  //===== Wait =====//
  /* for (i = 0; i < numProcesses; i = i + 1) {
    w = waitpid(pids[i], &status);
    fprintf(stdout,"%d\n", w);
  } */
  getchar();
  return (0);
}
