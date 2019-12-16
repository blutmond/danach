#include <spawn.h>
#include <string>
#include <string.h>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <memory>
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

void AssertWStatus(int wstatus) {
  if (WIFEXITED(wstatus)) {
    if (WEXITSTATUS(wstatus) == 0) return;
    fprintf(stderr, "exited, status=%d\n", WEXITSTATUS(wstatus));
  } else if (WIFSIGNALED(wstatus)) {
    fprintf(stderr, "killed by signal %d\n", WTERMSIG(wstatus));
  } else if (WIFSTOPPED(wstatus)) {
    fprintf(stderr, "stopped by signal %d\n", WSTOPSIG(wstatus));
  } else if (WIFCONTINUED(wstatus)) {
    fprintf(stderr, "continued\n");
  } else {
    fprintf(stderr, "Unknown wstatus: %d\n", wstatus);
  }
  exit(EXIT_FAILURE);
}

int WaitFor(pid_t cpid) {
  int wstatus;
  int w = waitpid(cpid, &wstatus, 0);
  if (w == -1) {
    fprintf(stderr, "waitpid_perror: %s\n", std::strerror(errno));
    exit(EXIT_FAILURE);
  }
  return wstatus;
}

void WaitForAssert(pid_t cpid) { AssertWStatus(WaitFor(cpid)); }

void Run(const char *argv[]) {
  for (int i = 0;argv[i]; ++i) {
    if (i != 0) printf(" ");
    printf("%s", argv[i]);
  }
  printf("\n");
  WaitForAssert(RunPid(argv));
}

void Run(std::vector<const char*> argv) {
  argv.push_back(nullptr);
  Run(argv.data());
}

pid_t RunPidWithPipe(const char *argv[], int out_fd, int err_fd,
                     std::vector<int> close_list) {
  pid_t pid;
  posix_spawn_file_actions_t action;
  posix_spawn_file_actions_init(&action);
  if (out_fd > 0) posix_spawn_file_actions_adddup2(&action, out_fd, 1);
  if (err_fd > 0) posix_spawn_file_actions_adddup2(&action, err_fd, 2);
  for (int fd : close_list) {
    posix_spawn_file_actions_addclose(&action, fd);
  }

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
  WaitForAssert(RunPidWithPipe(argv.data(), out_fd, -1, {}));
  close(out_fd);
}

int RunWithPipe(std::vector<const char*> argv,
                std::string* stdout_str,
                        std::string* stderr_str) {
  argv.push_back(nullptr);
  struct PipedFd {
    std::string* str = nullptr;
    int write_fd;
    int read_fd;

    explicit PipedFd(std::string* str) : str(str) {
      int pipefd[2];
      if (pipe(pipefd) != 0) {
        fprintf(stderr, "pipe error: %s\n", std::strerror(errno));
        exit(EXIT_FAILURE);
      }
      read_fd = pipefd[0];
      write_fd = pipefd[1];
    }
  };

  std::vector<std::unique_ptr<PipedFd>> pipes;
  auto GetFd = [&](std::string* str) -> PipedFd* {
    if (str) {
      for (auto& fd : pipes) {
        if (fd->str == str) return fd.get();
      }
      auto fd = std::make_unique<PipedFd>(str);
      auto* res = fd.get();
      pipes.emplace_back(std::move(fd));
      return res;
    }
    return nullptr;
  };
  PipedFd* stdout_fd = GetFd(stdout_str);
  PipedFd* stderr_fd = GetFd(stderr_str);

  std::vector<int> read_fds;
  for (auto& fd : pipes) read_fds.push_back(fd->read_fd);

  auto pid = RunPidWithPipe(argv.data(), stdout_fd ? stdout_fd->write_fd : -1,
                                         stderr_fd ? stderr_fd->write_fd : -1,
                                         read_fds);

  for (auto& fd : pipes) close(fd->write_fd);

  // Epoll if two fds...?
  for (auto& fd : pipes) {
    while (true) {
      size_t kReadLength = 2048;
      char buf[kReadLength];
      ssize_t res = read(fd->read_fd, &buf[0], kReadLength);
      if (res < 0) {
        fprintf(stderr, "pipe error: %s\n", std::strerror(errno));
        exit(EXIT_FAILURE);
      }
      fd->str->append(&buf[0], res);
      if (res == 0) break;
    }
  }

  return WaitFor(pid);
}
