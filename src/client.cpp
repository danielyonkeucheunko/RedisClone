#include "buffer.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <iostream>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

static void msg(const char *msg) { fprintf(stderr, "%s\n", msg); }

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static int32_t read_full(int fd, uint8_t *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) {
            return -1; // error, or unexpected EOF
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t write_all(int fd, const uint8_t *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1; // error
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

const size_t k_max_msg = 32 << 20; // likely larger than the kernel buffer

// the `query` function was simply splited into `send_req` and `read_res`.
static int32_t send_req(int fd, const uint8_t *text, size_t len) {
    if (len > k_max_msg) {
        return -1;
    }

    buffer_t wbuf{};
    if (buf_init(&wbuf, 16) != 0) {
        return -1; // error initializing
    }

    buf_append(&wbuf, (const uint8_t *)&len, 4);
    buf_append(&wbuf, text, len);
    return write_all(fd, wbuf.data_begin, buf_size(&wbuf));
}

static int32_t read_res(int fd) {
    // 4 bytes header
    buffer_t rbuf{};
    if (buf_init(&rbuf, 4) != 0) {
        return -1; // error initializing
    }

    errno = 0;
    int32_t err = read_full(fd, rbuf.data_begin, 4);
    if (err) {
        if (errno == 0) {
            msg("EOF");
        } else {
            msg("read() error");
        }
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf.data_begin, 4); // assume little endian
    if (len > k_max_msg) {
        msg("too long");
        std::cout << len << "\n";
        return -1;
    }

    // reply body
    buf_resize(&rbuf, 4 + len);
    err = read_full(fd, &rbuf.data_begin[4], len);
    if (err) {
        msg("read() error");
        return err;
    }

    // do something
    printf("len:%u data:%.*s\n", len, len < 100 ? len : 100,
           &rbuf.data_begin[4]);
    return 0;
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("connect");
    }

    // multiple pipelined requests

    std::vector<std::string> query_list = {
        "hello1",
        "hello2",
        "hello3",
        // a large message requires multiple event loop iterations
        std::string(k_max_msg, 'z'),
        "hello5",
    };

    printf("Client:\n");

    for (const std::string &s : query_list) {
        int32_t err = send_req(fd, (uint8_t *)s.data(), s.size());
        std::cout << s.size() << "\n";
        if (err) {
            goto L_DONE;
        }
    }
    for (size_t i = 0; i < query_list.size(); ++i) {
        int32_t err = read_res(fd);
        if (err) {
            goto L_DONE;
        }
    }

L_DONE:
    close(fd);
    return 0;
}
