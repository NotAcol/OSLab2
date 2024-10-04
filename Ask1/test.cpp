#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

void SigAction(int signal, siginfo_t *info, void *);

int main(int argc, char *argv[]) {
    // NOTE(acol): making read only file for 0 and 1
    creat("./.hello_there", S_IRUSR);

    // NOTE(acol): unpause with SIGCONT for 2
    struct sigaction act{};
    act.sa_sigaction = SigAction;
    act.sa_flags = SA_RESTART | SA_SIGINFO | SA_NOCLDWAIT;
    if (sigemptyset(&act.sa_mask)) return 1;
    if (sigaction(SIGCHLD, &act, NULL)) return 1;

    // NOTE(acol): environment variables for 3 and extra hints
    setenv("ANSWER", "42", 0);
    setenv("I_NEED_TECH_HINTS", "1", 0);

    // NOTE(acol): fifo for 4 with rw-r--r-- perms
    mkfifo("./magic_mirror", 0644);

    // NOTE(acol): creating an fd 99 for 5
    int fd = open("./test.cpp", O_APPEND);
    dup2(fd, 99);

    // NOTE(acol): making ping pong pipes for 6
    int pingfd[2];
    pipe(pingfd);
    dup2(pingfd[0], 33);
    dup2(pingfd[1], 34);

    int pongfd[2];
    pipe(pongfd);
    dup2(pongfd[0], 53);
    dup2(pongfd[1], 54);

    // NOTE(acol): hard link .hello_there to .hey_there for 7
    link("./.hello_there", "./.hey_there");

    // NOTE(acol): make sparce files bf00 to bf09 with user read write perms and
    // writing 16 chars at 1073741824 for 8
    char buff[]{"aaaaaaaaaaaaaaa"};

    std::string name{"bf0"};
    for (int i{}; i < 10; ++i) {
        fd = open((name + std::to_string(i)).c_str(), O_WRONLY | O_CREAT,
                  S_IRUSR | S_IWUSR);
        lseek(fd, 1073741824, SEEK_SET);
        write(fd, buff, 16);
    }

    pid_t cpid = fork();
    if (cpid < 0) return 1;

    if (!cpid) {
        execl("./riddle", "./riddle");
    } else {
        // NOTE(acol): CHILD PROROCESS DOESNT RAISE SIGCHLD WHEN IT PAUSES
        //             so I had to do this scuffed shit WHY????
        usleep(600);
        kill(cpid, SIGCONT);

        for (;;) {
            usleep(100);
        }
    }

    for (int i{}; i < 2; ++i) {
        close(pingfd[i]);
        close(pongfd[i]);
    }
    return 0;
}

void SigAction(int signal, siginfo_t *info, void *) {
    if (info->si_code == CLD_STOPPED || info->si_code == CLD_TRAPPED) {
        std::cout << "child paused\n";
        if (kill(info->si_pid, SIGCONT)) raise(SIGTERM);
    } else if (info->si_code == CLD_CONTINUED) {
        std::cout << "resumed\n";
    } else if (info->si_code == CLD_EXITED) {
        raise(SIGTERM);
    }
}
