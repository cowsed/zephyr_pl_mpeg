#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/multi_heap/shared_multi_heap.h>

void *my_malloc(size_t sz, int line);

void *my_realloc(void *oldp, size_t newsz, size_t oldsize);
void my_free(void *p);

#define PLM_MALLOC(sz) my_malloc(sz, __LINE__)
#define PLM_FREE(p) my_free(p)
// we get a lil silly with it. only ever called with self->capacity and we need
// that to make our own realloc
#define PLM_REALLOC(p, sz) my_realloc(p, sz, self->capacity)

#include "pl_mpeg/pl_mpeg.h"
#include <stddef.h>
#include <zephyr/fs/fs.h>

void plm_file_adapter_load_callback(plm_buffer_t *self, void *user);

void plm_file_adapter_seek_callback(plm_buffer_t *self, size_t offset,
                                    void *user);
size_t plm_file_adapter_tell_callback(plm_buffer_t *self, void *user);

struct plm_file_adapter {
  const char *filename;
  struct fs_file_t file;
};

plm_buffer_t *plm_file_adapter_init(const char *fname,
                                    struct plm_file_adapter *custom);