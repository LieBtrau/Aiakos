/*
  Ring buffer Library.
  Created by Pirmin Kalberer.
  Released into the public domain.
*/

#ifndef RingBuffer_h
#define RingBuffer_h
#include "Arduino.h"

class RingBuffer
{
private:
  byte size;
  word* buffer;
  byte start;
  byte end;
  byte cnt;

public:
  RingBuffer(byte size);
  ~RingBuffer();
  void push(word value);
  word pop();
  word peek();
  byte count();
};

#endif
