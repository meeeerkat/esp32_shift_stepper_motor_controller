
#include "shift_stepper_motor_controller.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
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
      if (motor->steps_todo != 0) {

        if (motor->steps_todo < 0) {
          motor->current_step--;
          motor->steps_todo++;
        }
        else {
          motor->current_step++;
          motor->steps_todo--;
        }

        // Inv of control bits so that motor 0's outputs are the first 4 pins of the first 8bit shift register
        // -> adding shift registers (to add motors) doesn't change the pins or ids for motors already in place
        command_bits |= stepper_command[motor->current_step%4] << (ssmc->sr->bits_nb - (id+1)*4);
      }
    }

    shift_register__send(ssmc->sr, command_bits);

    // TODO: use xTaskWait instead
    vTaskDelay(pdMS_TO_TICKS(ssmc->period));
  }
}




void shift_stepper_motor_controller__init(shift_stepper_motor_controller_t* ssmc, uint8_t motors_nb, shift_register_t* sr, uint16_t period) {
  ssmc->motors_nb = motors_nb;
  ssmc->sr = sr;
  ssmc->period = period;
  // current_steps and steps_todo all set to 0
  memset(ssmc->motors, 0, MAX_MOTORS_NB*sizeof(shift_stepper_motor_t));

  xTaskCreate(task, "task_shift_stepper_motor_controller", 2048, ssmc, 10, NULL);
}

void shift_stepper_motor_controller__move(shift_stepper_motor_controller_t* ssmc, uint8_t motor_id, int64_t steps_todo) {
  ssmc->motors[motor_id].steps_todo = steps_todo;
//  ssmc->motors[motor_id].callback = callback;
}


