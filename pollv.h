// conn.h
#ifndef POLLV_H
#define POLLV_H
#include <stddef.h>
#include <stdlib.h>
typedef struct
{
  struct pollfd *data;
  size_t size;
  size_t capacity;
} PollArgs;

void poll_args_init(PollArgs *v);
void poll_args_push(PollArgs *v, struct pollfd *data);
void poll_args_set(PollArgs *v, size_t index, struct pollfd *data);
struct pollfd *poll_args_get(PollArgs *v, size_t index);
void pollargs_free(PollArgs *v);

#endif // POLL_H
