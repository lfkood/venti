#include <stdio.h>
#include <string.h>

#include "audio_controller.h"
#include "audio_decoder_registry.h"
#include "audio_engine.h"
#include "host_storage_posix.h"

static bool supports_known_formats(audio_media_format_t format) {
  return format == AUDIO_MEDIA_FORMAT_MP3 || format == AUDIO_MEDIA_FORMAT_FLAC ||
         format == AUDIO_MEDIA_FORMAT_OGG || format == AUDIO_MEDIA_FORMAT_WAV ||
         format == AUDIO_MEDIA_FORMAT_AAC || format == AUDIO_MEDIA_FORMAT_OPUS ||
         format == AUDIO_MEDIA_FORMAT_M4A;
}

static bool host_decoder_open(void *decoder_context,
                              const audio_storage_api_t *storage_api,
                              void *storage_context, const audio_track_t *track,
                              audio_storage_file_t *opened_file,
                              audio_stream_info_t *stream_info) {
  (void)decoder_context;
  if (storage_api == NULL || storage_api->open == NULL || track == NULL ||
      opened_file == NULL || stream_info == NULL) {
    return false;
  }
  if (!storage_api->open(storage_context, track->path, opened_file)) {
    return false;
  }

  stream_info->sample_rate_hz = 48000;
  stream_info->channel_count = 2;
  stream_info->bits_per_sample = 16;
  return true;
}

static size_t host_decoder_decode(void *decoder_context, int16_t *pcm_buffer,
                                  size_t pcm_sample_capacity) {
  (void)decoder_context;
  (void)pcm_buffer;
  (void)pcm_sample_capacity;
  return 0;
}

static void host_decoder_close(void *decoder_context,
                               const audio_storage_api_t *storage_api,
                               void *storage_context,
                               audio_storage_file_t *opened_file) {
  (void)decoder_context;
  if (storage_api != NULL && storage_api->close != NULL && opened_file != NULL) {
    storage_api->close(storage_context, opened_file);
  }
}

static const audio_decoder_api_t host_decoder = {
    .name = "host_decoder_stub",
    .supports_format = supports_known_formats,
    .open = host_decoder_open,
    .decode_interleaved_pcm = host_decoder_decode,
    .close = host_decoder_close,
};

static const char *engine_state_text(audio_engine_state_t state) {
  switch (state) {
    case AUDIO_ENGINE_STATE_IDLE:
      return "IDLE";
    case AUDIO_ENGINE_STATE_READY:
      return "READY";
    case AUDIO_ENGINE_STATE_PLAYING:
      return "PLAYING";
    case AUDIO_ENGINE_STATE_PAUSED:
      return "PAUSED";
    case AUDIO_ENGINE_STATE_STOPPED:
      return "STOPPED";
    case AUDIO_ENGINE_STATE_ERROR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}

static void print_playlist(const audio_engine_t *engine) {
  printf("Tracks: %zu\n", engine->track_count);
  for (size_t i = 0; i < engine->track_count; i++) {
    printf("  %zu: %s\n", i, engine->tracks[i].path);
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <audio-directory>\n", argv[0]);
    return 1;
  }

  host_storage_context_t storage_context = host_storage_create_context(argv[1]);
  audio_engine_t engine;
  const audio_engine_config_t config = {
      .storage_api = &host_storage_api,
      .storage_context = &storage_context,
      .decoders = &host_decoder,
      .decoder_count = 1,
  };

  if (!audio_engine_init(&engine, &config)) {
    fprintf(stderr, "Failed to initialize engine\n");
    return 1;
  }

  printf("Audio host simulator ready. Commands: list, play, pause, stop, next, prev, select <n>\n");
  print_playlist(&engine);

  char line[96];
  while (fgets(line, sizeof(line), stdin) != NULL) {
    audio_command_t command;
    if (!audio_controller_parse_command(line, &command)) {
      printf("Invalid command\n");
      continue;
    }
    if (command.type == AUDIO_COMMAND_LIST) {
      print_playlist(&engine);
      continue;
    }

    const bool ok = audio_engine_handle_command(&engine, &command);
    printf("%s (state=%s track=%zu)\n", ok ? "ok" : "failed",
           engine_state_text(engine.state), engine.current_track_index);
  }

  audio_engine_stop(&engine);
  return 0;
}
