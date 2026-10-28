#include "audio_decoder_registry.h"

#include <stdbool.h>
#include <string.h>

static char to_lower_ascii(char c) {
  if (c >= 'A' && c <= 'Z') {
    return (char)(c + ('a' - 'A'));
  }
  return c;
}

static bool extension_equals(const char *extension, const char *value) {
  size_t i = 0;
  if (extension == NULL || value == NULL) {
    return false;
  }
  for (;;) {
    const char lhs = to_lower_ascii(extension[i]);
    const char rhs = to_lower_ascii(value[i]);
    if (lhs != rhs) {
      return false;
    }
    if (lhs == '\0') {
      return true;
    }
    i++;
  }
}

audio_media_format_t audio_media_format_from_path(const char *path) {
  if (path == NULL) {
    return AUDIO_MEDIA_FORMAT_UNKNOWN;
  }

  const char *dot = strrchr(path, '.');
  if (dot == NULL || *(dot + 1) == '\0') {
    return AUDIO_MEDIA_FORMAT_UNKNOWN;
  }
  const char *extension = dot + 1;

  if (extension_equals(extension, "mp3")) {
    return AUDIO_MEDIA_FORMAT_MP3;
  }
  if (extension_equals(extension, "flac")) {
    return AUDIO_MEDIA_FORMAT_FLAC;
  }
  if (extension_equals(extension, "ogg")) {
    return AUDIO_MEDIA_FORMAT_OGG;
  }
  if (extension_equals(extension, "wav")) {
    return AUDIO_MEDIA_FORMAT_WAV;
  }
  if (extension_equals(extension, "aac")) {
    return AUDIO_MEDIA_FORMAT_AAC;
  }
  if (extension_equals(extension, "opus")) {
    return AUDIO_MEDIA_FORMAT_OPUS;
  }
  if (extension_equals(extension, "m4a")) {
    return AUDIO_MEDIA_FORMAT_M4A;
  }

  return AUDIO_MEDIA_FORMAT_UNKNOWN;
}

const audio_decoder_api_t *audio_decoder_registry_find(
    const audio_decoder_api_t *decoders, size_t decoder_count,
    audio_media_format_t format) {
  if (decoders == NULL || decoder_count == 0) {
    return NULL;
  }

  for (size_t i = 0; i < decoder_count; i++) {
    if (decoders[i].supports_format != NULL && decoders[i].supports_format(format)) {
      return &decoders[i];
    }
  }
  return NULL;
}
