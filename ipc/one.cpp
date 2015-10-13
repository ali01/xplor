
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <unistd.h>

ssize_t write_fd(int socket, const void *buffer, ssize_t len, int sendfd) {
    struct iovec iov = {};
    iov.iov_base = const_cast<void*>(buffer);
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

        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
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


int main(int argc, char **argv) {
    const int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (fd < 0) {
        printf("socket fail\n");
        return -1;
    }

    char sun_path[] = "\0blue-ipc\0";

    struct sockaddr_un addr = {};
    addr.sun_family = AF_UNIX;
    memcpy(&addr.sun_path, sun_path, 10);

    socklen_t length = sizeof(sa_family_t) + 10;
    int ret = connect(fd, reinterpret_cast<struct sockaddr*>(&addr), length);
    if (ret < 0) {
        printf("connect fail\n");
        return -1;
    }

    std::string str = "hello, world!";
    write_fd(fd, str.data(), str.size(), 1);

    return 0;
}
