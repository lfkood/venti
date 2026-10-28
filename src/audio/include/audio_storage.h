#ifndef VENTI_AUDIO_STORAGE_H
#define VENTI_AUDIO_STORAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "audio_media.h"

typedef struct {
  void *handle;
  uint32_t size_bytes;
} audio_storage_file_t;

typedef struct {
  bool (*list_tracks)(void *context, audio_track_t *out_tracks, size_t max_tracks,
                      size_t *out_count);
  bool (*open)(void *context, const char *path, audio_storage_file_t *out_file);
  size_t (*read)(void *context, audio_storage_file_t *file, void *buffer,
                 size_t byte_count);
  bool (*seek)(void *context, audio_storage_file_t *file, uint32_t offset_bytes);
  void (*close)(void *context, audio_storage_file_t *file);
} audio_storage_api_t;

#endif
