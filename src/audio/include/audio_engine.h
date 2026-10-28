#ifndef VENTI_AUDIO_ENGINE_H
#define VENTI_AUDIO_ENGINE_H

#include <stdbool.h>
#include <stddef.h>

#include "audio_controller.h"
#include "audio_decoder.h"
#include "audio_media.h"
#include "audio_storage.h"

#define AUDIO_ENGINE_MAX_TRACKS 128

typedef enum {
  AUDIO_ENGINE_STATE_IDLE = 0,
  AUDIO_ENGINE_STATE_READY,
  AUDIO_ENGINE_STATE_PLAYING,
  AUDIO_ENGINE_STATE_PAUSED,
  AUDIO_ENGINE_STATE_STOPPED,
  AUDIO_ENGINE_STATE_ERROR
} audio_engine_state_t;

typedef struct {
  const audio_storage_api_t *storage_api;
  void *storage_context;
  void *decoder_context;
  const audio_decoder_api_t *decoders;
  size_t decoder_count;
} audio_engine_config_t;

typedef struct {
  audio_engine_state_t state;
  audio_engine_config_t config;
  audio_track_t tracks[AUDIO_ENGINE_MAX_TRACKS];
  size_t track_count;
  size_t current_track_index;
  const audio_decoder_api_t *active_decoder;
  audio_storage_file_t active_file;
  audio_stream_info_t active_stream;
  bool active_file_open;
} audio_engine_t;

bool audio_engine_init(audio_engine_t *engine, const audio_engine_config_t *config);
bool audio_engine_refresh_library(audio_engine_t *engine);
bool audio_engine_play(audio_engine_t *engine);
bool audio_engine_pause(audio_engine_t *engine);
bool audio_engine_stop(audio_engine_t *engine);
bool audio_engine_next(audio_engine_t *engine);
bool audio_engine_previous(audio_engine_t *engine);
bool audio_engine_select_track(audio_engine_t *engine, size_t track_index);
bool audio_engine_handle_command(audio_engine_t *engine,
                                 const audio_command_t *command);

#endif
