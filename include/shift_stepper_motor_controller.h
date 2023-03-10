#ifndef _SHIFT_STEPPER_MOTOR_CONTROLLER_H_
#define _SHIFT_STEPPER_MOTOR_CONTROLLER_H_

#include <stdint.h>
#include "shift_register.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


// 16*4 = 64 = max bits nb shift_registers can handle
#define MAX_MOTORS_NB 16


typedef struct {
  int64_t current_step, aimed_step;
//  void (*callback) (uint8_t);
} shift_stepper_motor_t;

typedef struct {
  uint8_t motors_nb;
  shift_register_t* sr;
  uint16_t period;
  shift_stepper_motor_t motors[MAX_MOTORS_NB];
  QueueHandle_t finished_movement_queue;
} shift_stepper_motor_controller_t;


extern void shift_stepper_motor_controller__init(shift_stepper_motor_controller_t* ssmc, uint8_t motors_nb, shift_register_t* sr, uint16_t period, size_t finished_movement_queue_length);
extern void shift_stepper_motor_controller__move(shift_stepper_motor_controller_t* ssmc, uint8_t motor_id, int64_t steps_todo);
extern void shift_stepper_motor_controller__moveto(shift_stepper_motor_controller_t* ssmc, uint8_t motor_id, int64_t aimed_position);
extern QueueHandle_t* shift_stepper_motor_controller_finished_movement_queue(shift_stepper_motor_controller_t* ssmc);

#endif
