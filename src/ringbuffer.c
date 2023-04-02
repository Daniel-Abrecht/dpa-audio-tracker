#define _GNU_SOURCE
#include <ringbuffer.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

struct ringbuffer {
  char* buffer;
  unsigned offset, size;
};

__attribute__((const))
static inline unsigned get_ringbuffer_size(void){
  const unsigned sz = sysconf(_SC_PAGESIZE);
  return (sz-1 + 4096) / sz * sz;
}

struct ringbuffer* ringbuffer_create(void){
  struct ringbuffer* rb = calloc(1,sizeof(struct ringbuffer));
  if(!rb){
    fprintf(stderr, "%s:%u: calloc failed (%d): %s\n", __FILE__, __LINE__, errno, strerror(errno));
    goto error;
  }

  const unsigned size = get_ringbuffer_size();

  const int memfd = memfd_create("ml666 json token emmiter ringbuffer", MFD_CLOEXEC);
  if(memfd == -1){
    fprintf(stderr, "%s:%u: memfd_create failed (%d): %s\n", __FILE__, __LINE__, errno, strerror(errno));
    goto error_calloc;
  }

  if(ftruncate(memfd, size) == -1){
    fprintf(stderr, "%s:%u: ftruncate failed (%d): %s\n", __FILE__, __LINE__, errno, strerror(errno));
    goto error_memfd;
  }

  // Allocate any 4 free pages |A|B|C|D|
  char*const mem = mmap(0, size*4, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if(mem == MAP_FAILED){
    fprintf(stderr, "%s:%u: mmap failed (%d): %s\n", __FILE__, __LINE__, errno, strerror(errno));
    goto error_memfd;
  }

  // Replace them with the same one, rw |E|B|C|D|
  if(mmap(mem, size, PROT_WRITE, MAP_SHARED | MAP_FIXED, memfd, 0) == MAP_FAILED){
    fprintf(stderr, "%s:%u: mmap failed (%d): %s\n", __FILE__, __LINE__, errno, strerror(errno));
    goto error_mmap;
  }

  // Replace them with the same one, rw |E|E|C|D|
  if(mmap(mem+size, size, PROT_WRITE, MAP_SHARED | MAP_FIXED, memfd, 0) == MAP_FAILED){
    fprintf(stderr, "%s:%u: mmap failed (%d): %s\n", __FILE__, __LINE__, errno, strerror(errno));
    goto error_mmap;
  }

  // Replace them with the same one, ro |E|E|E|D|
  if(mmap(mem+size*2, size, PROT_READ, MAP_SHARED | MAP_FIXED, memfd, 0) == MAP_FAILED){
    fprintf(stderr, "%s:%u: mmap failed (%d): %s\n", __FILE__, __LINE__, errno, strerror(errno));
    goto error_mmap;
  }

  // Replace them with the same one, ro |E|E|E|E|
  if(mmap(mem+size*3, size, PROT_READ, MAP_SHARED | MAP_FIXED, memfd, 0) == MAP_FAILED){
    fprintf(stderr, "%s:%u: mmap failed (%d): %s\n", __FILE__, __LINE__, errno, strerror(errno));
    goto error_mmap;
  }

  close(memfd);

  rb->buffer = mem;

  return rb;

error_mmap:
  munmap(mem, size*4);
error_memfd:
  close(memfd);
error_calloc:
  free(rb);
error:
  return 0;
}

struct buffer_ro ringbuffer_get_read_buffer(const struct ringbuffer* rb){
  const unsigned size = get_ringbuffer_size();
  return (struct buffer_ro){
    .length = rb->size,
    .v = &rb->buffer[size+rb->offset],
  };
}

void ringbuffer_discard(struct ringbuffer* rb, int count){
  const unsigned size = get_ringbuffer_size();
  unsigned offset = rb->offset;
  if(count < 0)
    count = 0;
  if((unsigned)count > rb->size)
    count = rb->size;
  rb->size -= count;
  offset += count;
  if(offset >= size)
    offset -= size;
  rb->offset = offset;
}

struct buffer_wo ringbuffer_get_write_buffer(const struct ringbuffer* rb){
  const unsigned size = get_ringbuffer_size();
  unsigned offset = rb->size + rb->offset;
  if(offset >= size)
    offset -= size;
  return (struct buffer_wo){
    .length = size - rb->size,
    .v = &rb->buffer[offset],
  };
}

void ringbuffer_commit(struct ringbuffer* rb, int count){
  const int size = get_ringbuffer_size();
  if(count < 0)
    return;
  if(count > size)
    count = size;
  count += rb->size;
  if(count > size)
    count = size;
  rb->size = count;
}

void ringbuffer_destroy(struct ringbuffer* rb){
  const unsigned size = get_ringbuffer_size();
  munmap(rb->buffer, size*4);
  free(rb);
}
