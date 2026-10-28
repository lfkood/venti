#ifndef VENTI_AUDIO_CONTROLLER_H
#define VENTI_AUDIO_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  AUDIO_COMMAND_NONE = 0,
  AUDIO_COMMAND_PLAY,
  AUDIO_COMMAND_PAUSE,
  AUDIO_COMMAND_STOP,
  AUDIO_COMMAND_NEXT,
  AUDIO_COMMAND_PREVIOUS,
  AUDIO_COMMAND_LIST,
  AUDIO_COMMAND_SELECT_TRACK
} audio_command_type_t;

typedef struct {
  audio_command_type_t type;
  uint32_t track_index;
} audio_command_t;

bool audio_controller_parse_command(const char *line, audio_command_t *out_command);

#endif
