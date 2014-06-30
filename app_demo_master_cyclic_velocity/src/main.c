/**
 * @file main.c
 * @brief Example Master App for Cyclic Synchronous Velocity (on PC)
 * @author Synapticon GmbH (www.synapticon.com)
 */

#include <ctrlproto_m.h>
#include <ecrt.h>
#include <stdio.h>
#include <stdbool.h>
#include <profile.h>
#include <drive_function.h>
#include <motor_define.h>
#include <sys/time.h>
#include <time.h>
#include "ethercat_setup.h"

enum {ECAT_SLAVE_0};

int main() {

    int target_velocity = 2000; //rpm
    int acceleration = 1000; //rpm/s
    int deceleration = 1000; //rpm/s

    int actual_velocity = 0; // rpm
    int actual_position; // ticks
    float actual_torque; // mNm
    int steps = 0;
    int velocity_ramp = 0; // rpm

    /* Initialize Ethercat Master */
    init_master(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize torque parameters */
    initialize_torque(ECAT_SLAVE_0, slv_handles);

    /* Initialize all connected nodes with Mandatory Motor Configurations (specified in config/motor/)*/
    init_nodes(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize the node specified with ECAT_SLAVE_0 with CSV configurations (specified in config/motor/)*/
    set_operation_mode(CSV, ECAT_SLAVE_0, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    /* Enable operation of node in CSV mode */
    enable_operation(ECAT_SLAVE_0, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    /* Initialize velocity profile parameters */
    steps = init_velocity_profile_params(target_velocity,
            actual_velocity, acceleration, deceleration, ECAT_SLAVE_0,
            slv_handles);

    /* Just for better printing result */
    printf("\n");
    system("setterm -cursor off");

    if (master_setup.op_flag) /*Check if the master is active*/
    {
        for (int step = 1; step < steps + 1; step++) {
            /* Update the process data (EtherCat packets) sent/received from the node */
            pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

            /* Generate target velocity steps */
            velocity_ramp = generate_profile_velocity(step, ECAT_SLAVE_0,
                    slv_handles);

            /* Send target velocity for the node specified by ECAT_SLAVE_0 */
            set_velocity_rpm(velocity_ramp, ECAT_SLAVE_0, slv_handles);

            /* Read actual node sensor values */
            actual_velocity
                    = get_velocity_actual_rpm(ECAT_SLAVE_0, slv_handles);
            actual_position = get_position_actual_ticks(ECAT_SLAVE_0,
                    slv_handles);
            actual_torque = get_torque_actual_mNm(ECAT_SLAVE_0, slv_handles);

            printf("\r    Velocity: %d    Position: %d    Torque: %f        ",
                    actual_velocity, actual_position, actual_torque);
        }
    }

    printf("\n");

    /* Quick stop velocity mode (for emergency) */
    quick_stop_velocity(ECAT_SLAVE_0, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    /* Regain control of node to continue after quick stop */
    renable_ctrl_quick_stop(CSV, ECAT_SLAVE_0, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    set_operation_mode(CSV, ECAT_SLAVE_0, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    enable_operation(ECAT_SLAVE_0, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    /* Shutdown node operations */
    shutdown_operation(CSV, ECAT_SLAVE_0, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    /* Just for better printing result */
    system("setterm -cursor on");

    return 0;
}

