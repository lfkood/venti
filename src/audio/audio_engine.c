#include "audio_engine.h"

#include <string.h>

#include "audio_decoder_registry.h"

static bool audio_engine_close_active_track(audio_engine_t *engine) {
  if (engine == NULL) {
    return false;
  }
  if (engine->active_file_open && engine->active_decoder != NULL &&
      engine->active_decoder->close != NULL) {
    engine->active_decoder->close(engine->config.decoder_context, engine->config.storage_api,
                                  engine->config.storage_context, &engine->active_file);
  }
  engine->active_file_open = false;
  engine->active_decoder = NULL;
  memset(&engine->active_file, 0, sizeof(engine->active_file));
  memset(&engine->active_stream, 0, sizeof(engine->active_stream));
  return true;
}

static bool audio_engine_open_current_track(audio_engine_t *engine) {
  if (engine == NULL || engine->current_track_index >= engine->track_count) {
    return false;
  }

  audio_track_t *track = &engine->tracks[engine->current_track_index];
  track->format = audio_media_format_from_path(track->path);
  const audio_decoder_api_t *decoder = audio_decoder_registry_find(
      engine->config.decoders, engine->config.decoder_count, track->format);
  if (decoder == NULL || decoder->open == NULL) {
    return false;
  }

  audio_storage_file_t file = {0};
  audio_stream_info_t stream = {0};
  if (!decoder->open(engine->config.decoder_context, engine->config.storage_api, engine->config.storage_context,
                     track, &file, &stream)) {
    return false;
  }

  engine->active_decoder = decoder;
  engine->active_file = file;
  engine->active_stream = stream;
  engine->active_file_open = true;
  return true;
}

bool audio_engine_init(audio_engine_t *engine, const audio_engine_config_t *config) {
  if (engine == NULL || config == NULL || config->storage_api == NULL ||
      config->storage_api->list_tracks == NULL) {
    return false;
  }

  memset(engine, 0, sizeof(*engine));
  engine->config = *config;
  engine->state = AUDIO_ENGINE_STATE_IDLE;
  return audio_engine_refresh_library(engine);
}

bool audio_engine_refresh_library(audio_engine_t *engine) {
  if (engine == NULL) {
    return false;
  }
  size_t listed_count = 0;
  if (!engine->config.storage_api->list_tracks(engine->config.storage_context,
                                               engine->tracks, AUDIO_ENGINE_MAX_TRACKS,
                                               &listed_count)) {
    engine->track_count = 0;
    engine->state = AUDIO_ENGINE_STATE_ERROR;
    return false;
  }

  engine->track_count = listed_count;
  engine->current_track_index = 0;
  engine->state = (listed_count > 0) ? AUDIO_ENGINE_STATE_READY : AUDIO_ENGINE_STATE_IDLE;
  return true;
}

bool audio_engine_play(audio_engine_t *engine) {
  if (engine == NULL || engine->track_count == 0) {
    return false;
  }
  if (engine->state == AUDIO_ENGINE_STATE_PLAYING) {
    return true;
  }

  audio_engine_close_active_track(engine);
  if (!audio_engine_open_current_track(engine)) {
    engine->state = AUDIO_ENGINE_STATE_ERROR;
    return false;
  }

  engine->state = AUDIO_ENGINE_STATE_PLAYING;
  return true;
}

bool audio_engine_pause(audio_engine_t *engine) {
  if (engine == NULL || engine->state != AUDIO_ENGINE_STATE_PLAYING) {
    return false;
  }
  engine->state = AUDIO_ENGINE_STATE_PAUSED;
  return true;
}

bool audio_engine_stop(audio_engine_t *engine) {
  if (engine == NULL) {
    return false;
  }
  audio_engine_close_active_track(engine);
  engine->state = (engine->track_count > 0) ? AUDIO_ENGINE_STATE_STOPPED
                                             : AUDIO_ENGINE_STATE_IDLE;
  return true;
}

bool audio_engine_select_track(audio_engine_t *engine, size_t track_index) {
  if (engine == NULL || track_index >= engine->track_count) {
    return false;
  }
  engine->current_track_index = track_index;
  if (engine->state == AUDIO_ENGINE_STATE_PLAYING ||
      engine->state == AUDIO_ENGINE_STATE_PAUSED) {
    return audio_engine_play(engine);
  }
  engine->state = AUDIO_ENGINE_STATE_READY;
  return true;
}

bool audio_engine_next(audio_engine_t *engine) {
  if (engine == NULL || engine->track_count == 0) {
    return false;
  }
  const size_t next_index = (engine->current_track_index + 1) % engine->track_count;
  return audio_engine_select_track(engine, next_index);
}

bool audio_engine_previous(audio_engine_t *engine) {
  if (engine == NULL || engine->track_count == 0) {
    return false;
  }
  const size_t previous_index =
      (engine->current_track_index == 0) ? (engine->track_count - 1)
                                         : (engine->current_track_index - 1);
  return audio_engine_select_track(engine, previous_index);
}

bool audio_engine_handle_command(audio_engine_t *engine,
                                 const audio_command_t *command) {
  if (engine == NULL || command == NULL) {
    return false;
  }

  switch (command->type) {
    case AUDIO_COMMAND_PLAY:
      return audio_engine_play(engine);
    case AUDIO_COMMAND_PAUSE:
      return audio_engine_pause(engine);
    case AUDIO_COMMAND_STOP:
      return audio_engine_stop(engine);
    case AUDIO_COMMAND_NEXT:
      return audio_engine_next(engine);
    case AUDIO_COMMAND_PREVIOUS:
      return audio_engine_previous(engine);
    case AUDIO_COMMAND_SELECT_TRACK:
      return audio_engine_select_track(engine, command->track_index);
    case AUDIO_COMMAND_LIST:
      return true;
    case AUDIO_COMMAND_NONE:
    default:
      return false;
  }
}
