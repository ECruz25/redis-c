#include "poll.h"
#include "pollv.h"
#include <stdlib.h>
#include <stdio.h>

void poll_args_init(PollArgs *v)
{
  v->size = 0;
  v->capacity = 16;
  v->data = malloc(v->capacity * sizeof(struct pollfd)); // Fix: sizeof(struct pollfd), not sizeof(struct pollfd *)
}

void poll_args_push(PollArgs *v, struct pollfd *data)
{
  if (v->size >= v->capacity)
  {
    v->capacity *= 2;
    v->data = realloc(v->data, v->capacity * sizeof(struct pollfd)); // Fix: sizeof(struct pollfd)
  }
  v->data[v->size++] = *data; // This is correct - copy the struct
}

void poll_args_set(PollArgs *v, size_t index, struct pollfd *data)
{
  if (index >= v->capacity)
  {
    while (v->capacity <= index)
    {
      v->capacity *= 2;
    }
    v->data = realloc(v->data, v->capacity * sizeof(struct pollfd)); // Fix: sizeof(struct pollfd)
  }
  if (index >= v->size)
  {
    v->size = index + 1;
  }
  v->data[index] = *data; // This is correct - copy the struct
}

struct pollfd *poll_args_get(PollArgs *v, size_t index)
{
  if (index >= v->size)
    return NULL;
  return &v->data[index];
}

void pollargs_free(PollArgs *v)
{
  if (v->data == NULL) {
    printf("No data to free\n");
    return; // Avoid double free
  }
  free(v->data);
  v->data = NULL;      // ← Add this line!
  v->size = 0;         // ← Add this line!
  v->capacity = 0;     // ← Add this line!
}