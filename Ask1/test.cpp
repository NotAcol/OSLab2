#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <poll.h>
#include <pthread.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cctype>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>

bool terminate{};

void SigAction(int signal, siginfo_t *info, void *);
int GetNum(char *buf, int &i, int n);

int main(int argc, char *argv[]) {
    // NOTE(acol): making read only file (0) (1)
    creat("./.hello_there", S_IRUSR);

    // NOTE(acol): unpause with SIGCONT (2)
    struct sigaction act{};
    act.sa_sigaction = SigAction;
    act.sa_flags = SA_RESTART | SA_SIGINFO | SA_NOCLDWAIT;
    if (sigemptyset(&act.sa_mask)) return 1;
    if (sigaction(SIGCHLD, &act, NULL)) return 1;

    // NOTE(acol): environment variables for (3) and extra hints
    setenv("ANSWER", "42", 0);
    setenv("I_NEED_TECH_HINTS", "1", 0);

    // NOTE(acol): fifo for 4 with rw-r--r-- perms
    mkfifo("./magic_mirror", 0644);

    // NOTE(acol): creating an fd 99 (5)
    int fd = open("./test.cpp", O_APPEND);
    dup2(fd, 99);

    // NOTE(acol): making ping pong pipes (6)
    int pingfd[2];
    pipe(pingfd);
    dup2(pingfd[0], 33);
    dup2(pingfd[1], 34);

    int pongfd[2];
    pipe(pongfd);
    dup2(pongfd[0], 53);
    dup2(pongfd[1], 54);

    // NOTE(acol): hard link .hello_there to .hey_there (7)
    link("./.hello_there", "./.hey_there");

    // NOTE(acol):  make sparce files bf00 to bf09 with user read write perms
    //              and writing 16 chars at 1073741824 for each one (8)
    char buf1[]{"aaaaaaaaaaaaaaa"};

    std::string name{"bf0"};
    for (int i{}; i < 10; ++i) {
        fd = open((name + std::to_string(i)).c_str(), O_WRONLY | O_CREAT,
                  S_IRUSR | S_IWUSR);
        lseek(fd, 1073741824, SEEK_SET);
        write(fd, buf1, 16);
        close(fd);
    }

    // NOTE(acol):  making an tcp server side for (9)
    int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    // NOTE(acol):  have to use same port like ssh, how do you dodge that on the
    //              internet ??
    sin.sin_port = htons(49842);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0) return 1;

    listen(sd, 1);

    // NOTE(acol): (10) and (11)
    creat("./secret", S_IRUSR | S_IWUSR);
    link("secret", "secret_number");

    int child_sdin_pipe[2];
    pipe(child_sdin_pipe);
    pid_t cpid = fork();
    if (cpid < 0) return 1;

    if (!cpid) {
        // NOTE(acol): making this pipe the stdin for child
        dup2(child_sdin_pipe[0], STDIN_FILENO);
        close(child_sdin_pipe[1]);
        execl("./riddle", "./riddle");
    } else {
        close(child_sdin_pipe[0]);
        // FIX(acol): CHILD PROROCESS DOESNT RAISE SIGCHLD WHEN IT PAUSES
        //            so I had to do this scuffed shit for (2)
        //            why????
        sleep(1);
        kill(cpid, SIGCONT);

        // NOTE(acol): putting accept from (9) here cause it's blocking
        struct sockaddr_in peer_addr;
        socklen_t peer_addr_len;
        int num1{}, num2{}, i{};
        char op{};
        char buf[100];

        int sockfd = accept(sd, (struct sockaddr *)&peer_addr, &peer_addr_len);
        usleep(100);
        int n = read(sockfd, buf, 100);
        for (; i < n && !isdigit(buf[i]); ++i) {
        }  // find first num

        for (; i < n; ++i) {
            if (buf[i] == '+' || buf[i] == '-' || buf[i] == '/' ||
                buf[i] == '*') {
                op = buf[i];
            } else if (isdigit(buf[i])) {
                if (num1 == 0) {
                    num1 = GetNum(buf, i, n);
                } else {
                    num2 = GetNum(buf, i, n);
                    switch (op) {
                        case '+': {
                            num1 += num2;
                        } break;
                        case '-': {
                            num1 -= num2;
                        } break;
                        case '*': {
                            num1 *= num2;
                        } break;
                        case '/': {
                            num1 /= num2;
                        } break;
                    }
                }
            }
        }
        write(sockfd, std::to_string(num1).c_str(),
              std::to_string(num1).length());

        usleep(100);
        memset(buf, 0, sizeof(buf));
        int secret_fd = open("./secret", O_RDONLY);
        std::string secret_answer;
        i = 0;
        read(secret_fd, buf, sizeof(buf));
        close(secret_fd);
        secret_fd = -1;
        for (; i < 100 && buf[i] != ':'; ++i) {
        }
        i += 2;
        while (i < 100 && buf[i] != '\n') {
            secret_answer += buf[i++];
        }
        std::cout << secret_answer << std::endl;
        secret_answer += '\n';
        write(child_sdin_pipe[1], secret_answer.c_str(),
              (int)(secret_answer.size()));

        // (11)
        while ((secret_fd = open("secret_number", O_RDONLY)) < 0);

        memset(buf, 0, sizeof(buf));
        secret_answer.clear();
        i = 0;
        usleep(100);
        int r = read(secret_fd, buf, sizeof(buf));
        for (; i < 100 && buf[i] != ':'; ++i) {
        }
        i += 2;
        while (i < 100 && buf[i] != '\n') {
            secret_answer += buf[i++];
        }
        secret_answer += '\n';
        std::cout << secret_answer << std::endl;
        write(child_sdin_pipe[1], secret_answer.c_str(),
              (int)(secret_answer.size()));
        close(secret_fd);

        while (!terminate) {
            usleep(10000);
        }

        for (int i{}; i < 2; ++i) {
            close(pingfd[i]);
            close(pongfd[i]);
        }
        close(sd);
        close(sockfd);

        name = "bf0";
        for (int i{}; i < 10; ++i) {
            unlink((name + std::to_string(i)).c_str());
        }

        unlink("./magic_mirror");
        unlink("./riddle.savegame");
        unlink("./.hey_there");
        unlink("./.hello_there");
        unlink("./secret");
    }
    return 0;
}

void SigAction(int signal, siginfo_t *info, void *) {
    if (info->si_code == CLD_STOPPED) {
        std::cout << "child paused\n";
        if (kill(info->si_pid, SIGCONT)) raise(SIGTERM);
    } else if (info->si_code == CLD_CONTINUED) {
        std::cout << "resumed\n";
    } else if (info->si_code == CLD_EXITED) {
        terminate = true;
    }
}

int GetNum(char *buf, int &i, int n) {
    int answer{};
    for (; i < n && isdigit(buf[i]); ++i) {
        answer = answer * 10 + buf[i] - '0';
    }
    return answer;
}
