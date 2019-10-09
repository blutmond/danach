#include <spawn.h>
#include <string>
#include <string.h>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

pid_t RunPid(const char *argv[]) {
  pid_t pid;

  int res = posix_spawn(&pid, argv[0], nullptr, nullptr,
                  (char *const *)argv, environ);
  if (res != 0) {
    fprintf(stderr, "spawn_perror: %s\n", std::strerror(errno));
    exit(EXIT_FAILURE);
  }
  return pid;
}

void WaitFor(pid_t cpid) {
  int wstatus;
  int w = waitpid(cpid, &wstatus, 0);
  if (w == -1) {
    fprintf(stderr, "waitpid_perror: %s\n", std::strerror(errno));
  } else if (WIFEXITED(wstatus)) {
    if (WEXITSTATUS(wstatus) == 0) return;
    fprintf(stderr, "exited, status=%d\n", WEXITSTATUS(wstatus));
  } else if (WIFSIGNALED(wstatus)) {
    fprintf(stderr, "killed by signal %d\n", WTERMSIG(wstatus));
  } else if (WIFSTOPPED(wstatus)) {
    fprintf(stderr, "stopped by signal %d\n", WSTOPSIG(wstatus));
  } else if (WIFCONTINUED(wstatus)) {
    fprintf(stderr, "continued\n");
  }
  exit(EXIT_FAILURE);
}

void Run(const char *argv[]) {
  for (int i = 0;argv[i]; ++i) {
    if (i != 0) printf(" ");
    printf("%s", argv[i]);
  }
  printf("\n");
  WaitFor(RunPid(argv));
}

void Run(std::vector<const char*> argv) {
  argv.push_back(nullptr);
  Run(argv.data());
}

pid_t RunPidWithPipe(const char *argv[], int out_fd) {
  pid_t pid;
  posix_spawn_file_actions_t action;
  posix_spawn_file_actions_init(&action);
  posix_spawn_file_actions_adddup2(&action, out_fd, 1);

  int res = posix_spawn(&pid, argv[0], &action, nullptr,
                  (char *const *)argv, environ);
  if (res != 0) {
    fprintf(stderr, "spawn_perror: %s\n", std::strerror(errno));
    exit(EXIT_FAILURE);
  }

  posix_spawn_file_actions_destroy(&action);
  return pid;
}

void RunWithPipe(std::vector<const char*> argv, const char* out_filename) {
  argv.push_back(nullptr);
  for (int i = 0;argv[i]; ++i) {
    if (i != 0) printf(" ");
    printf("%s", argv[i]);
  }
  printf(" > %s\n", out_filename);

  int out_fd = creat(out_filename, S_IRUSR | S_IWUSR);
  if (out_fd == -1) {
    fprintf(stderr, "failed to create: %s %s\n", out_filename, std::strerror(errno));
    exit(EXIT_FAILURE);
  }
  WaitFor(RunPidWithPipe(argv.data(), out_fd));
  close(out_fd);
}
