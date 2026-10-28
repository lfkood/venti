#ifndef VENTI_HOST_STORAGE_POSIX_H
#define VENTI_HOST_STORAGE_POSIX_H

#include "audio_storage.h"

typedef struct {
  const char *root_directory;
} host_storage_context_t;

extern audio_storage_api_t host_storage_api;
host_storage_context_t host_storage_create_context(const char *directory);

#endif
