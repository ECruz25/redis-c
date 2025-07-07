#include "conn.h"
#include <stdlib.h>
#include <string.h>

void conn_vector_init(ConnVector *v) {
  v->size = 0;
  v->capacity = 16;
  v->data = malloc(v->capacity * sizeof(Conn *));
}

void conn_vector_push(ConnVector *v, Conn *conn) {
  if (v->size >= v->capacity) {
    v->capacity *= 2;
    v->data = realloc(v->data, v->capacity * sizeof(Conn *));
  }
  v->data[v->size++] = conn;
}

void conn_vector_set(ConnVector *v, size_t index, Conn *conn) {
  if (index >= v->capacity) {
    while (v->capacity <= index) {
      v->capacity *= 2;
    }
    v->data = realloc(v->data, v->capacity * sizeof(Conn *));
  }
  if (index >= v->size) {
    v->size = index + 1;
  }
  v->data[index] = conn;
}

Conn *conn_vector_get(ConnVector *v, size_t index) {
  if (index >= v->size)
    return NULL;
  return v->data[index];
}

void conn_vector_free(ConnVector *v) { free(v->data); }

void conn_vector_resize(ConnVector *v, size_t new_size, uint8_t fill_value) {
  if (new_size > v->capacity) {
    while (v->capacity < new_size) {
      v->capacity *= 2;
    }
    v->data = realloc(v->data, v->capacity * sizeof(uint8_t));
  }

  if (new_size > v->size) {
    // Fill new elements with fill_value
    memset(v->data + v->size, fill_value, new_size - v->size);
  }

  v->size = new_size;
}
