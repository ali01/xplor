
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

ssize_t write_fd(int socket, void *buffer, ssize_t len, int sendfd) {
    struct iovec iov = {};
    iov.iov_base = buffer;
    iov.iov_len = len;

    struct msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (sendfd != -1) {
        union {
            struct cmsghdr cmsghdr;
            char   control[CMSG_SPACE(sizeof(int))];
        } cmsgu;

        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);

        struct cmsghdr *cmsg;
        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        *((int*)CMSG_DATA(cmsg)) = sendfd;
    }

    ssize_t ret = ::sendmsg(socket, &msg, 0);
    if (ret < 0) {
        printf("socketWriteFd: error sending message (%lu): %s\n", ret, std::strerror(errno));
        return -1;
    }

    return ret;
}

ssize_t read_fd(int socket, void *buffer, ssize_t len, int *recvfd) {
    ssize_t size;
    if (recvfd) {
        union {
            struct cmsghdr cmsghdr;
            char control[CMSG_SPACE(sizeof(int))];
        } cmsgu;

        struct iovec iov;
        iov.iov_base = buffer;
        iov.iov_len = len;

        struct msghdr msg;
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);

        size = recvmsg(socket, &msg, 0);
        if (size < 0) {
            fprintf(stderr, "error receiving message");
            exit(1);
        }

        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
            if (cmsg->cmsg_level != SOL_SOCKET) {
                fprintf(stderr, "invalid cmsg_level");
                exit(1);
            }

            if (cmsg->cmsg_type != SCM_RIGHTS) {
                fprintf(stderr, "invalid cmsg_type");
                exit(1);
            }

            *recvfd = *((int*)CMSG_DATA(cmsg));
            printf("received fd %d\n", *recvfd);

        } else {
            *recvfd = -1;
        }
    } else {
        size = read(socket, buffer, len);
        if (size < 0) {
            fprintf(stderr, "error receiving message");
            exit(1);
        }
    }

    return size;
}

void child(int socket) {
    printf("child running!\n");

    int fd;
    char buffer[16];
    size_t size = read_fd(socket, buffer, sizeof(buffer), &fd);
    if (size <= 0) {
        printf("error read_fd");
        exit(1);
    }

    printf("read %ld\n", size);
    if (fd != -1) {
        ::write(fd, buffer, size);
        close(fd);
    } else {
        printf("fuuuuu\n");
    }
}

void parent(int socket) {
    char buffer[] = "hella, world!\n";
    ssize_t size = write_fd(socket, buffer, 14, 1);
    printf("wrote %ld\n", size);
}

int main(int argc, char **argv) {
    setbuf(stdout, NULL);

    int sv[2];
    if (::socketpair(AF_UNIX, SOCK_SEQPACKET, 0, sv) < 0) {
        fprintf(stderr, "error creating socketpair\n");
        exit(1);
    }

    if (!::fork()) {
        printf("I'm the child!\n");
        child(sv[1]);
        exit(0);
    } else {
        printf("I'm the parent!\n");
        parent(sv[0]);
        wait(NULL);
    }

    return 0;
}
