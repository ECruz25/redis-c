#include "conn.h"
#include "pollv.h"
#include "vector.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>      // For struct addrinfo, getaddrinfo(), freeaddrinfo(), etc.
#include <netinet/in.h> // Defines sockaddr_in, in_addr, etc.
#include <poll.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/socket.h> // Defines AF_INET, SOCK_STREAM, etc.
#include <sys/types.h>
#include <unistd.h> // For close()

#define MYPORT "8000"

const size_t k_max_msg = 4096;

static void fd_set_nb(int fd)
{
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

static Conn *handle_accept(int fd)
{
  struct sockaddr_in client_addr = {};
  socklen_t addrlen = sizeof(client_addr);
  int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
  if (connfd < 0)
  {
    return NULL;
  }
  fd_set_nb(connfd);

  Conn *conn = malloc(sizeof(Conn));
  if (!conn)
  {
    close(connfd);
    return NULL;
  }
  conn->fd = connfd;
  conn->want_read = 1;
  conn->want_write = 0;
  conn->want_close = 0;
  byte_vector_init(&conn->incoming);
  byte_vector_init(&conn->outgoing);
  return conn;
}

static void buf_append(ByteVector *buf, const uint8_t *data, size_t len)
{
  size_t old_size = buf->size;
  byte_vector_resize(buf, old_size + len);
  memcpy(&buf->data[old_size], data, len);
}
// remove from the front
static void buf_consume(ByteVector *buf, size_t n)
{
  byte_vector_erase_range(buf, 0, n);
}

static bool try_one_request(Conn *conn)
{
  // 3. Try to parse the accumulated buffer.
  // Protocol: message header
  if (conn->incoming.size < 4)
  {
    printf("Not enough data for header: %zu bytes\n", conn->incoming.size);
    return false; // want read
  }
  uint32_t len = 0;
  memcpy(&len, conn->incoming.data, 4);
  if (len > k_max_msg)
  { // protocol error
    conn->want_close = true;
    printf("Message too long: %u bytes\n", len);
    return false; // want close
  }
  // Protocol: message body
  if (4 + len > conn->incoming.size)
  {
    printf("Not enough data for body: %zu bytes, expected %u bytes\n",
           conn->incoming.size, 4 + len);
    return false; // want read
  }
  const uint8_t *request = &conn->incoming.data[4];
  // 4. Process the parsed message.
  // ...
  // generate the response (echo)
  buf_append(&conn->outgoing, (const uint8_t *)&len, 4);
  buf_append(&conn->outgoing, request, len);
  // 5. Remove the message from `Conn::incoming`.
  buf_consume(&conn->incoming, 4 + len);
  printf("Processed request: %.*s\n", len, request);
  return true; // success
}

static void handle_write(Conn *conn)
{
  assert(conn->outgoing.size > 0);
  ssize_t rv = write(conn->fd, conn->outgoing.data, conn->outgoing.size);
  if (rv < 0)
  {
    conn->want_close = true; // error handling
    return;
  }
  // remove written data from `outgoing`
  buf_consume(&conn->outgoing, (size_t)rv);
  if (conn->outgoing.size == 0)
  { // all data written
    conn->want_read = true;
    conn->want_write = false;
  } // else: want write
}

static void handle_read(Conn *conn)
{
  // 1. Do a non-blocking read.
  uint8_t buf[64 * 1024];
  ssize_t rv = read(conn->fd, buf, sizeof(buf));
  if (rv <= 0)
  { // handle IO error (rv < 0) or EOF (rv == 0)
    conn->want_close = true;
    return;
  }
  buf_append(&conn->incoming, buf, (size_t)rv);
  try_one_request(conn);
  if (conn->outgoing.size > 0)
  { // has a response
    conn->want_read = false;
    conn->want_write = true;
  } // else: want read
}

int main()
{
  int status;
  int filedescriptor;
  struct addrinfo hints;
  struct addrinfo *servinfo;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "gai error: %s\n", gai_strerror(status));
    exit(1);
  }

  filedescriptor =
      socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

  int val = 1;
  setsockopt(filedescriptor, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  status = bind(filedescriptor, servinfo->ai_addr, servinfo->ai_addrlen);

  if (status != 0)
  {
    fprintf(stderr, "error binding %s", gai_strerror(status));
    exit(1);
  }

  status = listen(filedescriptor, 10);

  if (status != 0)
  {
    fprintf(stderr, "error listening %s", gai_strerror(status));
    exit(1);
  }
  printf("Before all nonsense\n");

  ConnVector fd2conn;
  conn_vector_init(&fd2conn);
  PollArgs pollargs;
  poll_args_init(&pollargs);
  printf("After init \n");

  while (1)
  {
    pollargs_free(&pollargs);

    struct pollfd pfd = {filedescriptor, POLLIN, 0};
    poll_args_push(&pollargs, &pfd);

    for (size_t i = 0; i < fd2conn.size; i++)
    {
      Conn *conn = fd2conn.data[i];
      if (!conn)
      {
        continue;
      }
      struct pollfd pfd = {conn->fd, POLLERR, 0};
      if (conn->want_read)
      {
        pfd.events |= POLLIN;
      }
      if (conn->want_write)
      {
        pfd.events |= POLLOUT;
      }
      poll_args_push(&pollargs, &pfd);
    }

    int rv = poll(pollargs.data, (nfds_t)pollargs.size, -1);

    if (rv < 0 && errno != EINTR)
    {
      continue;
    }

    if (rv < 0)
    {
      perror("poll");
      continue; // error handling
    }

    if (pollargs.data[0].revents)
    {
      Conn *acceptConn = handle_accept(filedescriptor);

      if (acceptConn)
      {
        // put it into the map
        if (fd2conn.size <= (size_t)acceptConn->fd)
        {
          conn_vector_resize(&fd2conn, acceptConn->fd + 1, 0);
        }
        fd2conn.data[acceptConn->fd] = acceptConn;
      }
    }

    for (size_t i = 1; i < pollargs.size; ++i)
    {
      uint32_t ready = pollargs.data[i].revents;
      if (fd2conn.size <= (size_t)pollargs.data[i].fd)
      {
        printf("Skipping fd %d, not in fd2conn\n", pollargs.data[i].fd);
        continue; // skip if fd is not in fd2conn
      }
      Conn *conn = fd2conn.data[pollargs.data[i].fd];
      if (!conn)
      {
        printf("Connection is NULL for fd %d, skipping\n", pollargs.data[i].fd);
        continue; // skip if conn is NULL
      }
      if (ready & POLLIN)
      {
        handle_read(conn); // application logic
      }
      if (ready & POLLOUT)
      {
        handle_write(conn); // application logic
      }

      if ((ready & POLLERR) || conn->want_close)
      {
        (void)close(conn->fd);
        fd2conn.data[conn->fd] = NULL;
        byte_vector_free(&conn->incoming);
        byte_vector_free(&conn->outgoing);
        free(conn);
      }
    }
  }
}