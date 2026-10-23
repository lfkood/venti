#ifndef VENTI_PID_H
#define VENTI_PID_H

typedef struct venti_pid venti_pid_t;

// In a wrapper due to clang builtins clashing with hardware/sync.h ones if both
// get included in a file The only issue mentioning them got rejected cuz the
// existing builtins don't have the correct signatures and fixing that would
// break existing code
venti_pid_t* venti_pid_init(float kp, float ki, float kd);

float venti_pid_step(venti_pid_t* pid, float error);
void venti_pid_reset(venti_pid_t* pid);

#endif