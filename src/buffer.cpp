#include "buffer.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int buf_init(buffer_t *buf, size_t capacity) {
    buf->buffer_begin = (uint8_t *)malloc(capacity);
    if (!buf->buffer_begin)
        return -1;

    buf->data_begin = buf->buffer_begin;
    buf->data_end = buf->buffer_begin;
    buf->buffer_end = buf->buffer_begin + capacity;
    return 0;
}

void buf_free(buffer_t *buf) {
    free(buf->buffer_begin);
    buf->buffer_begin = buf->buffer_end = buf->data_begin = buf->data_end =
        NULL;
}

size_t buf_size(buffer_t *buf) {
    if (!buf || !buf->data_begin || !buf->data_end) {
        return 0;
    }
    return (size_t)(buf->data_end - buf->data_begin);
}

size_t buf_capacity(buffer_t *buf) {
    return (size_t)(buf->buffer_end - buf->buffer_begin);
}

// remove from the front
void buf_consume(buffer_t *buf, size_t n) {
    size_t available = buf_size(buf);
    if (n > available)
        n = available;

    buf->data_begin += n;
    if (buf->data_begin == buf->data_end) {
        // reset to beginning when empty
        buf->data_begin = buf->data_end = buf->buffer_begin;
    }
}

int buf_resize(buffer_t *buf, size_t new_capacity) {
    size_t data_size = buf_size(buf);

    if (data_size > new_capacity) {
        return -1; // new capacity too small to hold data
    }

    uint8_t *new_buffer = (uint8_t *)malloc(new_capacity);
    if (!new_buffer)
        return -1;

    memcpy(new_buffer, buf->data_begin, data_size);

    // free the old buffer
    free(buf->buffer_begin);

    // update pointers
    buf->buffer_begin = new_buffer;
    buf->data_begin = buf->buffer_begin;
    buf->data_end = buf->data_begin + data_size;
    buf->buffer_end = buf->buffer_begin + new_capacity;

    return 0;
}

// append to the back
int buf_append(buffer_t *buf, const uint8_t *data, size_t len) {
    size_t data_size = buf_size(buf);
    size_t front_size = buf->data_begin - buf->buffer_begin;
    size_t back_size = buf->buffer_end - buf->data_end;

    if (back_size >= len) {
        // if there's enough space at the back append
        memcpy(buf->data_end, data, len);
        buf->data_end += len;
    } else if (front_size + back_size >= len) {
        // if there's enough space in the front and back shift data to the front
        // then append
        buf->data_begin =
            (uint8_t *)memmove(buf->buffer_begin, buf->data_begin, data_size);
        buf->data_end = buf->data_begin + data_size;
        memcpy(buf->data_end, data, len);
        buf->data_end += len;
    } else {
        // not enough space? allocate more then append
        size_t new_capacity = buf_capacity(buf) * 2;
        if (data_size + len > new_capacity)
            new_capacity = data_size + len;

        if (buf_resize(buf, new_capacity) != 0) {
            return -1; // error: new capacity too small
        }

        memcpy(buf->data_end, data, len);
        buf->data_end += len;
    }
    return 0;
}
