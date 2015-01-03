/*
  Ring buffer Library.
  Created by Pirmin Kalberer.
  Released into the public domain.
*/

#include "RingBuffer.h"

// AVR LibC Includes
#include <stdlib.h>

RingBuffer::RingBuffer(byte size)
    : size(size), start(0), end(0), cnt(0)
{
    buffer = (word*)calloc(size, sizeof(word));
}

RingBuffer::~RingBuffer()
{
    free(buffer);
}

void RingBuffer::push(word value)
{
    buffer[end] = value;
    if (++end > size) end = 0;
    if (cnt == size) {
        if (++start > size) start = 0;
    } else {
        ++cnt;
    }
}

word RingBuffer::pop()
{
    cli();
    word value = buffer[start];
    if (cnt > 0) {
        --cnt;
        if (++start > size) start = 0;
    }
    sei();
    return value;
}

word RingBuffer::peek()
{
    return buffer[start];
}

byte RingBuffer::count()
{
    return cnt;
}
