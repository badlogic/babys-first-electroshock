#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "mutex.h"

typedef enum {
	MOTOR_CLOSING,
	MOTOR_OPENING,
	MOTOR_IDLE
} motor_state_t;

typedef struct {
    mcugdx_mutex_t mutex;
    motor_state_t state;
    bool is_closed;
    double on_start_time;
    int pin_open;
    int pin_close;
} motor_t;

void motor_init(int pin_open, int pin_close);
void motor_update(void);
void motor_toggle(void);
bool motor_is_closed(void);
motor_state_t motor_state(void);

#ifdef __cplusplus
}
#endif