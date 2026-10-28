#include "audio_controller.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static char to_lower_ascii(char c) {
  if (c >= 'A' && c <= 'Z') {
    return (char)(c + ('a' - 'A'));
  }
  return c;
}

static bool word_equals(const char *lhs, const char *rhs) {
  size_t i = 0;
  if (lhs == NULL || rhs == NULL) {
    return false;
  }
  for (;;) {
    const char l = to_lower_ascii(lhs[i]);
    const char r = to_lower_ascii(rhs[i]);
    if (l != r) {
      return false;
    }
    if (l == '\0') {
      return true;
    }
    i++;
  }
}

static bool parse_u32(const char *text, uint32_t *out_value) {
  uint32_t value = 0;
  if (text == NULL || *text == '\0' || out_value == NULL) {
    return false;
  }
  for (size_t i = 0; text[i] != '\0'; i++) {
    if (text[i] < '0' || text[i] > '9') {
      return false;
    }
    value = value * 10u + (uint32_t)(text[i] - '0');
  }
  *out_value = value;
  return true;
}

bool audio_controller_parse_command(const char *line, audio_command_t *out_command) {
  if (line == NULL || out_command == NULL) {
    return false;
  }

  out_command->type = AUDIO_COMMAND_NONE;
  out_command->track_index = 0;

  char command[24] = {0};
  char argument[24] = {0};
  const int parsed = sscanf(line, "%23s %23s", command, argument);
  if (parsed < 1) {
    return false;
  }

  if (word_equals(command, "play")) {
    out_command->type = AUDIO_COMMAND_PLAY;
    return true;
  }
  if (word_equals(command, "pause")) {
    out_command->type = AUDIO_COMMAND_PAUSE;
    return true;
  }
  if (word_equals(command, "stop")) {
    out_command->type = AUDIO_COMMAND_STOP;
    return true;
  }
  if (word_equals(command, "next")) {
    out_command->type = AUDIO_COMMAND_NEXT;
    return true;
  }
  if (word_equals(command, "prev") || word_equals(command, "previous")) {
    out_command->type = AUDIO_COMMAND_PREVIOUS;
    return true;
  }
  if (word_equals(command, "list")) {
    out_command->type = AUDIO_COMMAND_LIST;
    return true;
  }
  if (word_equals(command, "select") && parsed >= 2) {
    uint32_t track_index = 0;
    if (!parse_u32(argument, &track_index)) {
      return false;
    }
    out_command->type = AUDIO_COMMAND_SELECT_TRACK;
    out_command->track_index = track_index;
    return true;
  }

  return false;
}
