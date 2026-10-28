#ifndef VENTI_AUDIO_MEDIA_H
#define VENTI_AUDIO_MEDIA_H

#include <stddef.h>
#include <stdint.h>

#define AUDIO_TRACK_PATH_MAX 192

typedef enum {
  AUDIO_MEDIA_FORMAT_UNKNOWN = 0,
  AUDIO_MEDIA_FORMAT_MP3,
  AUDIO_MEDIA_FORMAT_FLAC,
  AUDIO_MEDIA_FORMAT_OGG,
  AUDIO_MEDIA_FORMAT_WAV,
  AUDIO_MEDIA_FORMAT_AAC,
  AUDIO_MEDIA_FORMAT_OPUS,
  AUDIO_MEDIA_FORMAT_M4A
} audio_media_format_t;

typedef struct {
  char path[AUDIO_TRACK_PATH_MAX];
  audio_media_format_t format;
} audio_track_t;

#endif
