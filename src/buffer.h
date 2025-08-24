#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <string.h>
#include <unistd.h>

typedef struct Buffer {
    uint8_t *buffer_begin;
    uint8_t *buffer_end;
    uint8_t *data_begin;
    uint8_t *data_end;
} buffer_t;

int buf_init(buffer_t *buf, size_t capacity);

void buf_free(buffer_t *buf);

size_t buf_size(buffer_t *buf);

size_t buf_capacity(buffer_t *buf);

// remove from the front
void buf_consume(buffer_t *buf, size_t n);

int buf_resize(buffer_t *buf, size_t new_capacity);

// append to the back
int buf_append(buffer_t *buf, const uint8_t *data, size_t len);

#endif // BUFFER_H
