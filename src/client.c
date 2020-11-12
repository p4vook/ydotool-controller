#include "connection.h"
#include <stdio.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>

static inline void handle(int tested, char *message, int retcode) {
        if (tested != 0) {
                error(retcode, errno, message);
        }
}

int main(int argc, char *argv[]) {
        int data_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
        handle(data_socket == -1, "socket", 1);
        struct sockaddr_un address;
        address.sun_family = AF_UNIX;
        strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);
        handle(connect(data_socket, (const struct sockaddr *)&address, sizeof(struct sockaddr_un)), "connect", 1);
        int len_sum = 0;
        for (int arg = 1; arg < argc; ++arg) {
                len_sum += strlen(argv[arg]) + 1;
        }
        char buf[len_sum];
        char *current = buf;
        for (int arg = 1; arg < argc; ++arg) {
                memcpy(current, argv[arg], strlen(argv[arg]) + 1);
                current += strlen(argv[arg]) + 1;
        }
        handle(write(data_socket, buf, len_sum) != len_sum, "write", 1);
        close(data_socket);
        exit(EXIT_SUCCESS);
}
