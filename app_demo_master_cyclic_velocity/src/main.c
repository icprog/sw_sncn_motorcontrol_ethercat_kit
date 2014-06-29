/**
 * @file main.c
 * @brief Example Master App for Cyclic Synchronous Velocity (on PC)
 * @author Pavan Kanajar <pkanajar@synapticon.com>
 * @author Christian Holl <choll@synapticon.com>
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

int main() {

    int final_target_velocity = -2000; //rpm
    int acceleration = 100; //rpm/s
    int deceleration = 100; //rpm/s
    int steps = 0;
    int target_velocity = 0; // rpm
    int actual_velocity = 0; // rpm
    int actual_position; // ticks
    float actual_torque; // mNm
    int sdo_update = 1; // 1- yes / 0 - no
    int slave_number = 0;

    /* Initialize Ethercat Master */
    init_master(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize torque parameters */
    initialize_torque(slave_number, slv_handles);

    /* Initialize all connected nodes with Mandatory Motor Configurations (specified in config/motor/)*/
    init_nodes(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize the node specified with slave_number with CSV configurations (specified in config/motor/)*/
    set_operation_mode(CSV, slave_number, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    /* Enable operation of node in CSV mode */
    enable_operation(slave_number, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    /* Initialize velocity profile parameters */
    steps = init_velocity_profile_params(final_target_velocity,
            actual_velocity, acceleration, deceleration, slave_number,
            slv_handles);

    for (int i = 1; i < steps + 1; i++) {
        /* Update the process data (EtherCat packets) sent/received from the node */
        pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

        if (master_setup.op_flag) /*Check if the master is active*/
        {
            if (i < steps) {
                /* Generate target velocity steps */
                target_velocity = generate_profile_velocity(i, slave_number,
                        slv_handles);

                /* Send target velocity for the node specified by slave_number */
                set_velocity_rpm(target_velocity, slave_number, slv_handles);

                /* Read actual node sensor values */
                actual_velocity = get_velocity_actual_rpm(slave_number,
                        slv_handles);
                actual_position = get_position_actual_ticks(slave_number,
                        slv_handles);
                actual_torque
                        = get_torque_actual_mNm(slave_number, slv_handles);
                printf("\033[2J\033[1;1H\n\n"); //clears the terminal
                printf("    Velocity: %d\n\n", actual_velocity);
                printf("    Position: %d\n\n", actual_position);
                printf("    Torque: %f\n\n", actual_torque);
            }
        }
    }

    /* Quick stop velocity mode (for emergency) */
    quick_stop_velocity(slave_number, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    /* Regain control of node to continue after quick stop */
    renable_ctrl_quick_stop(CSV, slave_number, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    set_operation_mode(CSV, slave_number, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    enable_operation(slave_number, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    /* Shutdown node operations */
    shutdown_operation(CSV, slave_number, &master_setup, slv_handles,
            TOTAL_NUM_OF_SLAVES);

    return 0;
}

