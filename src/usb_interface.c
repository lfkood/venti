#include "usb_interface.h"

#include "FreeRTOS.h"
#include "fan_command.h"
#include "freertos_init.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "semphr.h"
#include "tachometer.h"
#include "task.h"
#include "venti_board_defs.h"
#include "venti_defs.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  uint32_t index;
  uint32_t target;
  uint32_t rpm;
  float voltage;
  fan_control_select_t command;
  fan_control_type_t status;
} fan_statistics_t;

typedef struct {
  char data[OUTPUT_BUFFER_SIZE];
  size_t length;
} terminal_output_t;

// Serialize complete cursor/echo operations against the statistics redraw.
static SemaphoreHandle_t terminal_mutex;

static void stdio_rx_task(void* pvParameters);
static void process_line(const char* line);
static void print_statistics_tx(const fan_statistics_t* fan_stats);

static void append_output(terminal_output_t* output, const char* format, ...)
    __attribute__((format(printf, 2, 3)));

static void append_output(terminal_output_t* output, const char* format, ...) {
  size_t available = sizeof(output->data) - output->length;
  va_list args;
  va_start(args, format);
  int count = vsnprintf(output->data + output->length, available, format, args);
  va_end(args);
  if (count > 0) {
    output->length += (size_t)count < available ? (size_t)count : available - 1;
  }
}

static void write_output(const terminal_output_t* output) {
  stdio_put_string(output->data, (int)output->length, false, false);
}

static void reset_prompt(void) {
  char prompt[32];
  int length = snprintf(prompt, sizeof(prompt), "\033[%u;1H\033[2K > ",
                        (unsigned)(FAN_COUNT + 3));
  stdio_put_string(prompt, length, false, false);
}

void stdio_interface(void* pvParameters) {
  (void)pvParameters;
  fan_statistics_t fan_stats[FAN_COUNT] = {0};
  for (uint32_t i = 0; i < FAN_COUNT; ++i) {
    fan_stats[i].index = i;
    fan_stats[i].command = SPEED_SELECT_OFF;
    fan_stats[i].status = CONTROL_SET_VOLTAGE;
  }

  terminal_mutex = xSemaphoreCreateMutex();
  if (terminal_mutex == NULL) {
    stdio_puts("Failed to create terminal mutex");
    vTaskDelete(NULL);
    return;
  }
  reset_prompt();
  if (xTaskCreate(stdio_rx_task, "usb_rx", 2048, NULL,
                  tskINTERFACE_PRIORITY, NULL) != pdPASS) {
    stdio_puts("Failed to create USB receive task");
  }
  for (;;) {
    for (uint32_t i = 0; i < FAN_COUNT; i++) {
      uint32_t notified_rpm;
      if (xTaskNotifyWaitIndexed(i, 0, 0, &notified_rpm, 0) == pdPASS) {
        fan_stats[i].rpm = notified_rpm;
      }
      fan_state_notification_t notified_state;
      if (xTaskNotifyWaitIndexed(FAN_INTERFACE_STATE_INDEX(i), 0, 0,
                                 &notified_state.as_notification, 0) == pdPASS) {
        fan_stats[i].target = notified_state.as_state.speed_value;
        fan_stats[i].command = notified_state.as_state.control_select;
        fan_stats[i].status = notified_state.as_state.control_type;
      }
    }
    xSemaphoreTake(terminal_mutex, portMAX_DELAY);
    print_statistics_tx(fan_stats);
    xSemaphoreGive(terminal_mutex);
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

static void stdio_rx_task(void* pvParameters) {
  (void)pvParameters;
  char buffer[RX_LINE_LENGTH];
  size_t pos = 0;
  bool overflow = false;
  bool previous_cr = false;
  bool connected = false;

  for (;;) {
    bool now_connected = stdio_usb_connected();
    if (!now_connected || !connected) {
      // Discard partial commands on an observed disconnect or new session. The
      // startup prompt may have been written before the host opened the port.
      pos = 0;
      overflow = false;
      previous_cr = false;
      if (now_connected) {
        xSemaphoreTake(terminal_mutex, portMAX_DELAY);
        reset_prompt();
        xSemaphoreGive(terminal_mutex);
      }
    }
    connected = now_connected;
    // Bound each batch so a continuous stream also yields to other tasks.
    for (size_t received = 0; connected && received < RX_LINE_LENGTH; ++received) {
      int ch = stdio_getchar_timeout_us(0);
      if (ch < 0) break;

      xSemaphoreTake(terminal_mutex, portMAX_DELAY);
      if (ch == '\n' && previous_cr) {
        previous_cr = false;
      } else if (ch == '\r' || ch == '\n') {
        if (overflow) {
          process_line(""); // Clear the previous response.
          stdio_put_string("Input too long; command discarded\r\n", -1, false, false);
        } else {
          buffer[pos] = '\0';
          process_line(buffer);
        }
        pos = 0;
        overflow = false;
        previous_cr = ch == '\r';
        reset_prompt();
      } else {
        previous_cr = false;
        // Once truncated, discard the entire line, including editing keys.
        if (!overflow) {
          if (ch == '\b' || ch == 127) {
            if (pos > 0) {
              --pos;
              stdio_put_string("\b \b", 3, false, false);
            }
          } else if ((ch >= 32 && ch <= 126) || ch == '\t') {
            if (pos < sizeof(buffer) - 1) {
              buffer[pos++] = ch == '\t' ? ' ' : (char)ch;
              stdio_putchar_raw(buffer[pos - 1]);
            } else {
              overflow = true;
            }
          }
        }
      }
      xSemaphoreGive(terminal_mutex);
    }
    // Pico stdio services USB in its background IRQ. Polling avoids binding a
    // FreeRTOS ISR notification callback to a particular core or stdio driver.
    TickType_t delay = pdMS_TO_TICKS(10);
    vTaskDelay(delay > 0 ? delay : 1);
  }
}

static bool parse_size_t(const char* str, size_t min, size_t max, size_t* value) {
  if (str == NULL || value == NULL || *str == '\0' || min > max) return false;
  size_t result = 0;
  for (const char* p = str; *p != '\0'; ++p) {
    if (*p < '0' || *p > '9') return false;
    size_t digit = (size_t)(*p - '0');
    if (result > max / 10 || (result == max / 10 && digit > max % 10)) return false;
    result = result * 10 + digit;
  }
  if (result < min) return false;
  *value = result;
  return true;
}

static bool parse_mode(const char* str, fan_control_type_t* mode) {
  if (strcmp(str, "pwm") == 0) {
    *mode = CONTROL_SET_PWM;
  } else if (strcmp(str, "voltage") == 0) {
    *mode = CONTROL_SET_VOLTAGE;
  } else {
    return false;
  }
  return true;
}

static int tokenize(const char* line, char tokens[5][16]) {
  int count = 0;
  while (*line) {
    while (isspace((unsigned char)*line)) ++line;
    if (!*line) break;
    if (count == 5) return -1;
    size_t length = 0;
    while (*line && !isspace((unsigned char)*line)) {
      // Reject long tokens instead of treating their suffix as another token.
      if (length == 15) return -1;
      tokens[count][length++] = *line++;
    }
    tokens[count++][length] = '\0';
  }
  return count;
}

static void print_usage(terminal_output_t* output) {
  append_output(output,
      "Usage:\r\n"
      "  fan <index> off\r\n"
      "  fan <index> auto [pwm|voltage]\r\n"
      "  fan <index> pwm <0..4095> [pwm|voltage]\r\n"
      "  fan <index> rpm <1..max> [pwm|voltage]\r\n");
}

static void process_line(const char* line) {
  terminal_output_t output = {0};
  append_output(&output, "\033[%u;1H", (unsigned)(FAN_COUNT + 5));
  for (unsigned i = 0; i < 6; ++i) {
    append_output(&output, i < 5 ? "\033[2K\033[1B" : "\033[2K");
  }
  append_output(&output, "\033[%u;1H", (unsigned)(FAN_COUNT + 5));

  char tokens[5][16];
  int argc = tokenize(line, tokens);
  if (argc == 0) {
    write_output(&output);
    return;
  }
  if (argc < 3 || strcmp(tokens[0], "fan") != 0) goto usage;

  size_t index;
  if (!parse_size_t(tokens[1], 0, FAN_COUNT - 1, &index)) {
    append_output(&output, "Invalid fan index\r\n");
    write_output(&output);
    return;
  }

  fan_control_notification_t notification = {
      .as_command = {SPEED_SELECT_OFF, CONTROL_UNCHANGED, 0}};
  const char* action = tokens[2];
  bool numeric = false;
  if (strcmp(action, "off") == 0) {
    if (argc != 3) goto usage;
  } else if (strcmp(action, "auto") == 0) {
    if (argc > 4) goto usage;
    notification.as_command.control_select = SPEED_SELECT_AUTO;
    if (argc == 4 && !parse_mode(tokens[3], &notification.as_command.control_type)) goto usage;
  } else if (strcmp(action, "pwm") == 0 || strcmp(action, "rpm") == 0) {
    if (argc < 4) goto usage;
    bool is_pwm = strcmp(action, "pwm") == 0;
    size_t speed;
    if (!parse_size_t(tokens[3], is_pwm ? 0 : 1,
                      is_pwm ? 4095 : MAX_RPM[index], &speed) || speed > UINT16_MAX) goto usage;
    notification.as_command.control_select = is_pwm ? SPEED_SELECT_PWM : SPEED_SELECT_PID;
    notification.as_command.control_type = is_pwm ? CONTROL_SET_PWM : CONTROL_SET_VOLTAGE;
    notification.as_command.speed_value = (uint16_t)speed;
    if (argc == 5 && !parse_mode(tokens[4], &notification.as_command.control_type)) goto usage;
    numeric = true;
  } else {
    goto usage;
  }

  if (FAN_CONTROL[index] == NULL ||
      xTaskNotifyIndexed(FAN_CONTROL[index], FAN_CONTROL_COMMAND_INDEX,
                         notification.as_notification, eSetValueWithOverwrite) != pdPASS) {
    append_output(&output, "Fan controller unavailable\r\n");
  } else if (numeric) {
    append_output(&output, "Fan %zu %s %u\r\n", index, action,
                   (unsigned)notification.as_command.speed_value);
  } else {
    append_output(&output, "Fan %zu %s\r\n", index, action);
  }
  write_output(&output);
  return;

usage:
  print_usage(&output);
  write_output(&output);
}

static void print_statistics_tx(const fan_statistics_t* fan_stats) {
  terminal_output_t output = {0};
  append_output(&output, "\033[s\033[1;1H\033[2K%-8s %-10s %-10s %-9s %-8s\r\n",
                 "Index", "Target", "RPM", "Control", "Command");
  for (uint32_t i = 0; i < FAN_COUNT; ++i) {
    const char* status = "UNKNOWN";
    const char* command = "UNKNOWN";
    switch (fan_stats[i].status) {
      case CONTROL_SET_PWM: status = "PWM"; break;
      case CONTROL_SET_VOLTAGE: status = "VOLTAGE"; break;
      case CONTROL_UNCHANGED: status = "UNCHANGED"; break;
    }
    switch (fan_stats[i].command) {
      case SPEED_SELECT_PID: command = "PID"; break;
      case SPEED_SELECT_PWM: command = "PWM"; break;
      case SPEED_SELECT_AUTO: command = "AUTO"; break;
      case SPEED_SELECT_OFF: command = "OFF"; break;
    }
    append_output(&output, "\033[2K%-8" PRIu32 " %-10" PRIu32 " %-10" PRIu32 " %-9s %-8s\r\n",
                   fan_stats[i].index, fan_stats[i].target, fan_stats[i].rpm, status, command);
  }
  append_output(&output, "\033[2K-------------------------------------------------\r\n > ");
  // Move past the prompt without erasing the user's partially typed command.
  append_output(&output, "\033[%u;1H\033[2K-------------------------------------------------\033[u",
                 (unsigned)(FAN_COUNT + 4));
  write_output(&output);
}
