#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stddef.h>

#define wronly

struct buffer_ro {
  unsigned length;
  union {
    const void* v;
    const char* c8;
    const unsigned char* u8;
    const signed char* s8;
  };
};

struct buffer_wo {
  unsigned length;
  union {
    wronly void* v;
    wronly char* c8;
    wronly unsigned char* u8;
    wronly signed char* s8;
  };
};

struct buffer {
  union {
    struct {
      unsigned length;
      union {
        void* v;
        char* c8;
        unsigned char* u8;
        signed char* s8;
      };
    };
    struct buffer_ro ro;
    struct buffer_wo wo;
  };
};

struct ringbuffer;
struct ringbuffer* ringbuffer_create(void);
void ringbuffer_destroy(struct ringbuffer* rb);

struct buffer_ro ringbuffer_get_read_buffer(const struct ringbuffer* rb);
void ringbuffer_discard(struct ringbuffer* rb, int count);
struct buffer_wo ringbuffer_get_write_buffer(const struct ringbuffer* rb);
void ringbuffer_commit(struct ringbuffer* rb, int count);

#endif
