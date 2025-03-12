#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void sig_handler(int signum, siginfo_t* info, void* myact) {
    if (signum == SIGINT)
        printf("Got a common signal SIGINT\n");
    else
        printf("Got a realtime signal SIGRTMIN\n");
}

int main() {
    struct sigaction act;
    sigset_t newmask, oldmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGINT);
    sigaddset(&newmask, SIGRTMIN);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    act.sa_sigaction = sig_handler;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGINT, &act, NULL) < 0 || sigaction(SIGRTMIN, &act, NULL) < 0) {
        perror("sigaction error");
        return 1;
    }

    printf("myprocesspid=%d\n", getpid());
    sleep(60);
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
    return 0;
}
