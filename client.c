#include <assert.h>
#include <errno.h>
#include <netdb.h> // For struct addrinfo, getaddrinfo(), freeaddrinfo(), etc.
#include <netinet/in.h> // Defines sockaddr_in, in_addr, etc.
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/socket.h> // Defines AF_INET, SOCK_STREAM, etc.
#include <sys/types.h>
#include <unistd.h> // For close()

#define MYPORT "8000"

const size_t k_max_msg = 4096;
static int32_t read_full(int fd, char *buf, size_t n) {
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

static int32_t write_all(int fd, const char *buf, size_t n) {
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
static int32_t query(int fd, const char *text) {
  uint32_t len = (uint32_t)strlen(text);
  if (len > k_max_msg) {
    printf("too long %i", len);
    return -1;
  }
  printf("client says: %s\n", text);
  // send request
  char wbuf[4 + k_max_msg];
  memcpy(wbuf, &len, 4); // assume little endian
  memcpy(&wbuf[4], text, len);
  int32_t err;
  printf("sending %i bytes\n", 4 + len);
  if ((err = write_all(fd, wbuf, 4 + len))) {
    printf("write() error");
    return err;
  }
  printf("write() ok\n");
  // 4 bytes header
  char rbuf[4 + k_max_msg + 1];
  errno = 0;
  err = read_full(fd, rbuf, 4);
  printf("read() %i bytes\n", 4);
  if (err) {
    printf(errno == 0 ? "EOF" : "read() error");
    return err;
  }
  uint32_t recv_len = 0;
  memcpy(&recv_len, rbuf, 4); // assume little endian
  if (recv_len > k_max_msg) {
    printf("too long %i", recv_len);
    return -1;
  }
  // reply body
  err = read_full(fd, &rbuf[4], recv_len);
  if (err) {
    printf("read() error");
    return err;
  }
  // do something
  printf("server says: %.*s\n", recv_len, &rbuf[4]);
  return 0;
}

int main() {
  int status;
  int filedescriptor;
  struct addrinfo hints;
  struct addrinfo *clientinfo;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(NULL, MYPORT, &hints, &clientinfo)) != 0) {
    fprintf(stderr, "gai error %s \n", gai_strerror(status));
    exit(1);
  }

  filedescriptor = socket(clientinfo->ai_family, clientinfo->ai_socktype,
                          clientinfo->ai_protocol);
  if (filedescriptor < 0) {
    fprintf(stderr, "error creating socket %s \n",
            gai_strerror(filedescriptor));
    exit(1);
  }
  printf("Socket created with file descriptor: %d\n", filedescriptor);

  int rv = connect(filedescriptor, clientinfo->ai_addr, clientinfo->ai_addrlen);
  if (rv) {
    fprintf(stderr, "error connecting %s \n", gai_strerror(rv));
    exit(1);
  }
  printf("Connected to server.\n");

  int32_t err = query(filedescriptor, "hello1");
  if (err) {
    goto L_DONE;
  }
  err = query(filedescriptor, "hello2");
  if (err) {
    goto L_DONE;
  }
L_DONE:
  close(filedescriptor);
  freeaddrinfo(clientinfo);
}
