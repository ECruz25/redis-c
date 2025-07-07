// vector.h
#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h> // optional if using bool
#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t *data;
  size_t size;
  size_t capacity;
} ByteVector;

void byte_vector_init(ByteVector *v);
void byte_vector_push(ByteVector *v, uint8_t byte);
void byte_vector_free(ByteVector *v);
uint8_t *byte_vector_begin(ByteVector *v);
uint8_t *byte_vector_end(ByteVector *v);
void byte_vector_insert(ByteVector *v, size_t index, uint8_t byte);
void byte_vector_erase(ByteVector *v, size_t index);
void byte_vector_erase_range(ByteVector *v, size_t from, size_t to);
void byte_vector_resize(ByteVector *v, size_t new_size);

#endif // VECTOR_H
