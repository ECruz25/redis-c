// conn.h
#ifndef CONN_H
#define CONN_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h> // optional if using bool
#include "vector.h"

#define CONN_INIT {-1, 0, 0, 0, {0}, {0}}

typedef struct
{
  int fd;
  int want_read;
  int want_write;
  int want_close;
  ByteVector incoming;
  ByteVector outgoing;
} Conn;

typedef struct
{
  Conn **data;
  size_t size;
  size_t capacity;
} ConnVector;

void conn_vector_init(ConnVector *v);
void conn_vector_push(ConnVector *v, Conn *conn);
void conn_vector_set(ConnVector *v, size_t index, Conn *conn);
Conn *conn_vector_get(ConnVector *v, size_t index);
void conn_vector_free(ConnVector *v);
void conn_vector_resize(ConnVector *v, size_t new_size, uint8_t fill_value);

#endif // CONN_H
