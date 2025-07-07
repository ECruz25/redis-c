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

int main() {
  int status;
  int filedescriptor, connfd;
  struct addrinfo hints;
  struct addrinfo *servinfo;
  struct sockaddr_storage their_addr;
  socklen_t addr_size;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "gai error: %s\n", gai_strerror(status));
    exit(1);
  }

  filedescriptor =
      socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

  printf("Socket created with file descriptor: %d\n", filedescriptor);

  status = bind(filedescriptor, servinfo->ai_addr, servinfo->ai_addrlen);

  if (status != 0) {
    fprintf(stderr, "error binding %s", gai_strerror(status));
    exit(1);
  }

  status = listen(filedescriptor, 10);

  if (status != 0) {
    fprintf(stderr, "error listening %s", gai_strerror(status));
    exit(1);
  }

  while (1) {
    connfd = accept(filedescriptor, (struct sockaddr *)&their_addr, &addr_size);
    if (connfd < 0)
      continue;
    close(connfd);
  }
}
