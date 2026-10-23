#include "pid.h"

#include "arm_math.h"
#include "stdint.h"
#include "venti_board_defs.h"
#include "venti_defs.h"

struct venti_pid {
  arm_pid_instance_f32 cmsis;
};

typedef struct venti_pid venti_pid_t;
static venti_pid_t controllers[FAN_COUNT*2];
static size_t controller_count = 0;

venti_pid_t* venti_pid_init(float kp, float ki, float kd) {
  if (controller_count >= FAN_COUNT) {
    return NULL;
  }

  venti_pid_t* pid = &controllers[controller_count];
  controller_count++;

  pid->cmsis = (arm_pid_instance_f32){
      .Kp = kp,
      .Ki = ki,
      .Kd = kd,
  };

  arm_pid_init_f32(&pid->cmsis, 1);
  return pid;
}

float venti_pid_step(venti_pid_t* pid, float error) {
  return arm_pid_f32(&pid->cmsis, error);
}

void venti_pid_reset(venti_pid_t* pid) { arm_pid_reset_f32(&pid->cmsis); }