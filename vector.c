// vector.c
#include "vector.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void byte_vector_init(ByteVector *v)
{
  v->size = 0;
  v->capacity = 16;
  v->data = malloc(v->capacity * sizeof(uint8_t));
}

void byte_vector_push(ByteVector *v, uint8_t byte)
{
  if (v->size >= v->capacity)
  {
    v->capacity *= 2;
    v->data = realloc(v->data, v->capacity * sizeof(uint8_t));
  }
  v->data[v->size++] = byte;
}

void byte_vector_free(ByteVector *v) {
    if (v->data) {
        free(v->data);
        v->data = NULL;  // Prevent double free
        v->size = 0;
        v->capacity = 0;
    }
}

uint8_t *byte_vector_begin(ByteVector *v) { return v->data; }

uint8_t *byte_vector_end(ByteVector *v) { return v->data + v->size; }

void byte_vector_insert(ByteVector *v, size_t index, uint8_t byte)
{
  if (index > v->size)
  {
    return; // or handle error
  }
  if (v->size >= v->capacity)
  {
    v->capacity *= 2;
    v->data = realloc(v->data, v->capacity * sizeof(uint8_t));
  }
  for (size_t i = v->size; i > index; --i)
  {
    v->data[i] = v->data[i - 1];
  }
  v->data[index] = byte;
  ++v->size;
}

void byte_vector_erase(ByteVector *v, size_t index)
{
  if (index >= v->size)
    return; // out of bounds

  // Shift elements left to fill the gap
  for (size_t i = index; i + 1 < v->size; i++)
  {
    v->data[i] = v->data[i + 1];
  }

  v->size--;
}

void byte_vector_erase_range(ByteVector *v, size_t from, size_t to)
{
  if (from >= to || to > v->size)
    return;

  size_t range = to - from;
  for (size_t i = from; i + range < v->size; i++)
  {
    v->data[i] = v->data[i + range];
  }

  v->size -= range;
}

void byte_vector_resize(ByteVector *v, size_t new_size)
{
  if (new_size > v->capacity)
  {
    // Need to grow the capacity
    size_t new_capacity = v->capacity;
    if (new_capacity == 0)
    {
      new_capacity = 1;
    }

    // Double capacity until it's big enough
    while (new_capacity < new_size)
    {
      new_capacity *= 2;
    }

    // Reallocate memory
    uint8_t *new_data = realloc(v->data, new_capacity * sizeof(uint8_t));
    if (!new_data)
    {
      // Handle allocation failure
      return;
    }

    v->data = new_data;
    v->capacity = new_capacity;
  }

  // If shrinking, optionally zero out the freed space
  if (new_size < v->size)
  {
    memset(v->data + new_size, 0, v->size - new_size);
  }

  v->size = new_size;
}