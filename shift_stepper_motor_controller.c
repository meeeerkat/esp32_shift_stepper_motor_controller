
#include "shift_stepper_motor_controller.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/gpio.h"
#include "freertos/task.h"





uint8_t stepper_command[] = {
  0b1100,
  0b0110,
  0b0011,
  0b1001
};


void task(void* args) {
  shift_stepper_motor_controller_t* ssmc = (shift_stepper_motor_controller_t*) args;

  while (1) {
    uint64_t command_bits = 0;
    for (uint8_t id=0; id < ssmc->motors_nb; id++) {
      shift_stepper_motor_t* motor = &ssmc->motors[id];
      const int64_t steps_todo = motor->aimed_step - motor->current_step;
      if (steps_todo != 0) {

        if (abs(steps_todo) == 1)
          xQueueSend(ssmc->finished_movement_queue, &id, 0);

        if (steps_todo < 0)
          motor->current_step--;
        else
          motor->current_step++;

        // Inv of control bits so that motor 0's outputs are the first 4 pins of the first 8bit shift register
        // -> adding shift registers (to add motors) doesn't change the pins or ids for motors already in place
        command_bits |= stepper_command[((motor->current_step%4) + 4)%4] << (ssmc->sr->bits_nb - (id+1)*4);
      }
    }

    shift_register__send(ssmc->sr, command_bits);

    // TODO: use xTaskWait instead
    vTaskDelay(pdMS_TO_TICKS(ssmc->period));
  }
}




void shift_stepper_motor_controller__init(shift_stepper_motor_controller_t* ssmc, uint8_t motors_nb, shift_register_t* sr, uint16_t period, size_t finished_movement_queue_length) {
  ssmc->motors_nb = motors_nb;
  ssmc->sr = sr;
  ssmc->period = period;
  // current_steps and steps_todo all set to 0
  memset(ssmc->motors, 0, MAX_MOTORS_NB*sizeof(shift_stepper_motor_t));
  ssmc->finished_movement_queue = xQueueCreate(finished_movement_queue_length, sizeof(uint8_t));

  xTaskCreate(task, "task_shift_stepper_motor_controller", 2048, ssmc, 10, NULL);
}

void shift_stepper_motor_controller__move(shift_stepper_motor_controller_t* ssmc, uint8_t motor_id, int64_t steps_todo) {
  shift_stepper_motor_t* motor = &ssmc->motors[motor_id];
  motor->aimed_step = motor->current_step + steps_todo;
}

void shift_stepper_motor_controller__moveto(shift_stepper_motor_controller_t* ssmc, uint8_t motor_id, int64_t aimed_position) {
  shift_stepper_motor_t* motor = &ssmc->motors[motor_id];
  motor->aimed_step = aimed_position;
}

QueueHandle_t* shift_stepper_motor_controller_finished_movement_queue(shift_stepper_motor_controller_t* ssmc) {
  return &ssmc->finished_movement_queue;
}

