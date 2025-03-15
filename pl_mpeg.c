#include <stddef.h>
#define PLM_NO_STDIO
#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg_adaptor.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pl_mpeg_adaptor, LOG_LEVEL_INF);

void my_free(void *p) { shared_multi_heap_free(p); }

void *my_malloc(size_t sz, int line) {
  // void *p = malloc(sz);
  void *p = shared_multi_heap_aligned_alloc(SMH_REG_ATTR_EXTERNAL, 32, sz);
  printf("line %d wanted %d got %p\n", line, sz, p);
  return p;
}

void *my_realloc(void *oldp, size_t newsz, size_t oldsz) {
  void *np = shared_multi_heap_aligned_alloc(SMH_REG_ATTR_EXTERNAL, 32, newsz);
  printf("rewanted %d from %dgot %p\n", newsz, oldsz, np);
  memcpy(np, oldp, oldsz);
  shared_multi_heap_free(oldp);
  return np;
}

void plm_file_adapter_load_callback(plm_buffer_t *self, void *user) {
  struct plm_file_adapter *custom_data = (struct plm_file_adapter *)user;
  printf("Load Callback called\n");
  if (self->discard_read_bytes) {
    plm_buffer_discard_read_bytes(self);
  }

  size_t bytes_available = self->capacity - self->length;
  printf("avail %d\n", bytes_available);
  // k_msleep(30);
  size_t bytes_read =
      fs_read(&custom_data->file, self->bytes + self->length, bytes_available);
  self->length += bytes_read;
  printf("read %d\n", bytes_read);
  if (bytes_read == 0) {
    self->has_ended = true;
  }
}
void plm_file_adapter_seek_callback(plm_buffer_t *self, size_t offset,
                                    void *user) {
  printf("Seek Callback called\n");

  struct plm_file_adapter *custom_data = (struct plm_file_adapter *)user;
  fs_seek(&custom_data->file, offset, FS_SEEK_SET);
}
size_t plm_file_adapter_tell_callback(plm_buffer_t *self, void *user) {
  printf("Tell Callback called\n");

  struct plm_file_adapter *custom_data = (struct plm_file_adapter *)user;

  return fs_tell(&custom_data->file);
}

plm_buffer_t *plm_file_adapter_init(const char *fname,
                                    struct plm_file_adapter *custom) {
  custom->filename = fname;
  fs_file_t_init(&custom->file);
  int ret = fs_open(&custom->file, custom->filename, FS_O_READ);
  if (ret < 0) {
    LOG_ERR("Error opening plm file adapter: %d", ret);
    return NULL;
  }
  fs_seek(&custom->file, 0, FS_SEEK_END);
  size_t len = fs_tell(&custom->file);
  fs_seek(&custom->file, 0, FS_SEEK_SET);

  plm_buffer_t *buf = plm_buffer_create_with_callbacks(
      plm_file_adapter_load_callback, plm_file_adapter_seek_callback,
      plm_file_adapter_tell_callback, len, (void *)custom);
  buf->discard_read_bytes = true;
  return buf;
}
