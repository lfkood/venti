#ifndef VENTI_AUDIO_DECODER_REGISTRY_H
#define VENTI_AUDIO_DECODER_REGISTRY_H

#include <stddef.h>

#include "audio_decoder.h"

audio_media_format_t audio_media_format_from_path(const char *path);
const audio_decoder_api_t *audio_decoder_registry_find(
    const audio_decoder_api_t *decoders, size_t decoder_count,
    audio_media_format_t format);

#endif
