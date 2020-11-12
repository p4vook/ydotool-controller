#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <linux/un.h>
#include "connection.h"

static inline void handle(int tested, char *message, int retcode) {
        if (tested != 0) {
                error(retcode, errno, message);
        }
}

int main() {
        int fd_listener = socket(AF_UNIX, SOCK_SEQPACKET, 0);
        handle(fd_listener == -1, "socket", 1);
        int option = 1;
        int p = 0;
        handle(setsockopt(fd_listener, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)), "setsockopt", 1);
        struct sockaddr_un address;
        memset(&address, 0, sizeof(struct sockaddr_un));
        address.sun_family = AF_UNIX;
        strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);
        handle(bind(fd_listener, (const struct sockaddr*)&address, sizeof(struct sockaddr_un)), "bind", 1);
        handle(listen(fd_listener, 16), "listen", 1);
        handle(chown(SOCKET_PATH, -1, 10), "chown", 1);
        handle(chmod(SOCKET_PATH, 660), "chmod", 1);
        printf("Listening on socket %s\n", address.sun_path);
        int fd_client;
        char buf[BUFFER_SIZE];
        char *args[BUFFER_SIZE + 1];
        while ((fd_client = accept(fd_listener, NULL, NULL)) != -1) {
                int rc = recv(fd_client, buf, sizeof(buf) - 1, MSG_WAITALL);
                char finished_prev = 1;
                args[0] = "ydotool";
                int current_arg = 1;
                for (unsigned i = 0; i < rc; ++i) {
                        if (buf[i] == '\0') {
                                finished_prev = 1;
                                continue;
                        }
                        if (finished_prev == 1) {
                                args[current_arg] = buf + i;
                                ++current_arg;
                                finished_prev = 0;
                        }
                }
                buf[rc] = '\0';
                if (rc == 0) {
                        continue;
                }
                args[current_arg] = NULL;
                if (fork() == 0) {
                        execvp("ydotool", args);
                } else {
                        wait(NULL);
                }
        }
        shutdown(fd_listener, SHUT_RDWR);
}
