#ifndef VENTI_AUDIO_DECODER_H
#define VENTI_AUDIO_DECODER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "audio_media.h"
#include "audio_storage.h"

typedef struct {
  uint32_t sample_rate_hz;
  uint8_t channel_count;
  uint8_t bits_per_sample;
} audio_stream_info_t;

typedef struct {
  const char *name;
  bool (*supports_format)(audio_media_format_t format);
  bool (*open)(void *decoder_context, const audio_storage_api_t *storage_api,
               void *storage_context, const audio_track_t *track,
               audio_storage_file_t *opened_file, audio_stream_info_t *stream_info);
  size_t (*decode_interleaved_pcm)(void *decoder_context, int16_t *pcm_buffer,
                                   size_t pcm_sample_capacity);
  void (*close)(void *decoder_context, const audio_storage_api_t *storage_api,
                void *storage_context, audio_storage_file_t *opened_file);
} audio_decoder_api_t;

#endif
