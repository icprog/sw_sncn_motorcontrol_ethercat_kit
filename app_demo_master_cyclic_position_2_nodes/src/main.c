
/**
 * \file main.c
 * \brief Example Master App for Cyclic Synchronous Position (on PC)
 * \author Pavan Kanajar <pkanajar@synapticon.com>
 * \author Christian Holl <choll@synapticon.com>
 * \version 1.0
 * \date 10/04/2014
 */

/*
 * Copyright (c) 2014, Synapticon GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Execution of this software or parts of it exclusively takes place on hardware
 *    produced by Synapticon GmbH.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the Synapticon GmbH.
 *
 */

#include <ctrlproto_m.h>
#include <ecrt.h>
#include "ethercat_setup.h"
#include <stdio.h>
#include <stdbool.h>
#include "profile.h"
#include "drive_function.h"
#include <motor_define.h>
#include <sys/time.h>
#include <time.h>
#include  <signal.h>

/* Only here for interrupt signaling */
bool break_loop = false;

/* Interrupt signal handler */
void  INThandler(int sig)
{
     signal(sig, SIG_IGN);
     break_loop = true;
     signal(SIGINT, INThandler);
}


int main()
{

    int acceleration_slave_1 = 350;     // rpm/s
    int acceleration_slave_2 = 350;     // rpm/s

    int deceleration_slave_1 = 350;     // rpm/s
    int deceleration_slave_2 = 350;     // rpm/s

    int velocity_slave_1 = 350;         // rpm
    int velocity_slave_2 = 350;         // rpm

    int actual_position_slave_1 = 0;    // ticks
    int actual_position_slave_2 = 0;    // ticks

    int zero_position_slave_1 = 0;    // ticks
    int zero_position_slave_2 = 0;    // ticks

    int target_position_slave_1 = 0;    // ticks
    int target_position_slave_2 = 0;    // ticks

    int actual_velocity_slave_1 = 0;    // rpm
    int actual_velocity_slave_2 = 0;    // rpm

    float actual_torque_slave_1;        // mNm
    float actual_torque_slave_2;        // mNm

    int steps_slave_1 = 0;
    int steps_slave_2 = 0;

    int inc_slave_1 = 1;
    int inc_slave_2 = 1;

    int next_target_position_slave_1 = 0;     // ticks
    int next_target_position_slave_2 = 0;     // ticks

    #define  SLAVE_1 0
    #define  SLAVE_2 1

    int one_rotation = 3 * 4096 * 1; //pole pairs * interpolation constant * gear ratio
    bool absolute_position_taken = false;

    bool new_target = true;
    int delay_inc = 0;

    /* Initialize EtherCAT Master */
    init_master(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize torque parameters */
    initialize_torque(SLAVE_1, slv_handles);
    initialize_torque(SLAVE_2, slv_handles);

    /* Initialize all connected nodes with Mandatory Motor Configurations (specified in motor_config)*/
    init_nodes(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize the node specified with slave_number with CSP configurations (specified under motor_config)*/
    set_operation_mode(CSP, SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
    set_operation_mode(CSP, SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Enable operation of node in CSP mode */
    enable_operation(SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
    enable_operation(SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize position profile parameters */
    initialize_position_profile_limits(SLAVE_1, slv_handles);
    initialize_position_profile_limits(SLAVE_2, slv_handles);


    while(1)
    {
        /* Update the process data (EtherCat packets) sent/received from the node */
        pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

        /* catch interrupt signal */
        signal(SIGINT, INThandler);

        if(master_setup.op_flag && !break_loop)    /*Check if the master is active*/
        {
            if (new_target) { //has to be done only once for a new target value

                /* Read Actual Position from the node for initialization */
                if (!absolute_position_taken) {
                    for (int i = 0; i < 3; i++)
                    {
                        zero_position_slave_1 = get_position_actual_ticks(SLAVE_1, slv_handles);
                        zero_position_slave_2 = get_position_actual_ticks(SLAVE_2, slv_handles);
                        pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
                        printf("taking abs position\n");
                    }
                    absolute_position_taken = true;
                    printf("abs positions are taken: \n%i\n%i\n", zero_position_slave_1, zero_position_slave_2);
                }

                /* Setup Target Position */
                target_position_slave_1 =  zero_position_slave_1 + one_rotation * 5;
                target_position_slave_2 =  zero_position_slave_1 + one_rotation * 5;

                /* Read Actual Position */
                actual_position_slave_1 = get_position_actual_ticks(SLAVE_1, slv_handles);
                actual_position_slave_2 = get_position_actual_ticks(SLAVE_2, slv_handles);

                /* Compute steps needed for the target position */
                steps_slave_1 = init_position_profile_params(target_position_slave_1, actual_position_slave_1,
                                velocity_slave_1, acceleration_slave_1, deceleration_slave_1, SLAVE_1, slv_handles);
                steps_slave_2 = init_position_profile_params(target_position_slave_2, actual_position_slave_2,
                                velocity_slave_2, acceleration_slave_2, deceleration_slave_2, SLAVE_2, slv_handles);

                printf("\nsteps_slave_1 %d target_slave_1 %d actual_slave_1 %d\n",
                        steps_slave_1, target_position_slave_1, actual_position_slave_1);
                printf("steps_slave_2 %d target_slave_2 %d actual_slave_2 %d\n\n",
                        steps_slave_2, target_position_slave_2, actual_position_slave_2);
                new_target = false;
            }

            if(inc_slave_1 < steps_slave_1)
            {
                /* Generate target position steps */
                next_target_position_slave_1 =  generate_profile_position(inc_slave_1, SLAVE_1, slv_handles);

                /* Send target position for the node specified by slave_number */
                set_position_ticks(next_target_position_slave_1, SLAVE_1, slv_handles);
                inc_slave_1 = inc_slave_1 + 1;
            }
            if(inc_slave_2 < steps_slave_2)
            {
                /* Generate target position steps */
                next_target_position_slave_2 =  generate_profile_position(inc_slave_2, SLAVE_2, slv_handles);

                /* Send target position for the node specified by slave_number */
                set_position_ticks(next_target_position_slave_2, SLAVE_2, slv_handles);
                inc_slave_2 = inc_slave_2 + 1;
            }
            if(inc_slave_1 >= steps_slave_1 && inc_slave_2 >= steps_slave_2)
            {
              delay_inc++;
              if(delay_inc > 500)//some delay to hold the position
              {
                  /* Set a new target position */
                  one_rotation = -one_rotation;

                  /* Reset increments */
                  inc_slave_1 = 1;
                  inc_slave_2 = 1;
                  delay_inc = 0;

                  /* Enable ramp calculation for new target position */
                  new_target = true;
              }
            }

            /* Read actual node sensor values */
            actual_position_slave_1 = get_position_actual_ticks(SLAVE_1, slv_handles);
            actual_velocity_slave_1 = get_velocity_actual_rpm(SLAVE_1, slv_handles);
            actual_torque_slave_1 = get_torque_actual_mNm(SLAVE_1, slv_handles);
            printf("actual position slave 1 %d actual velocity slave 1 %d actual_torque slave 1 %f\n",
                    actual_position_slave_1, actual_velocity_slave_1, actual_torque_slave_1);

            actual_position_slave_2 = get_position_actual_ticks(SLAVE_2, slv_handles);
            actual_velocity_slave_2 = get_velocity_actual_rpm(SLAVE_2, slv_handles);
            actual_torque_slave_2 = get_torque_actual_mNm(SLAVE_2, slv_handles);
            printf("actual position slave 2 %d actual velocity slave 2 %d actual_torque slave 2 %f\n\n",
                    actual_position_slave_2, actual_velocity_slave_2, actual_torque_slave_2);

        }
        else if (break_loop){
            break;
        }

    }

    /* Quick stop position mode (for emergency) */
    quick_stop_position(SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
    quick_stop_position(SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Regain control of node to continue after quick stop */
    renable_ctrl_quick_stop(CSP, SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES); //after quick-stop
    renable_ctrl_quick_stop(CSP, SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES); //after quick-stop

    set_operation_mode(CSP, SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
    set_operation_mode(CSP, SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    enable_operation(SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
    enable_operation(SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Shutdown node operations */
    shutdown_operation(CSP, SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
    shutdown_operation(CSP, SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    return 0;
}

