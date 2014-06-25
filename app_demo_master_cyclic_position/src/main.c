
/**
 * @file main.c
 * @brief Example Master App for Cyclic Synchronous Position (on PC)
 * @author Pavan Kanajar <pkanajar@synapticon.com>
 * @author Christian Holl <choll@synapticon.com>
 */

#include <ctrlproto_m.h>
#include <ecrt.h>
#include <ethercat_setup.h>
#include <stdio.h>
#include <stdbool.h>
#include <profile.h>
#include <drive_function.h>
#include <motor_define.h>
#include <sys/time.h>
#include <time.h>


int main()
{
    int flag = 0;

    int acceleration = 350;             // rpm/s
    int deceleration = 350;             // rpm/s
    int velocity = 350;                 // rpm
    int actual_position = 0;            // ticks
    int target_position = 0;            // ticks
    int actual_velocity = 0;            // rpm
    float actual_torque;                // mNm
    int steps = 0;
    int i = 1;
    int position_ramp = 0;

    int slave_number = 0;

    /* Initialize EtherCAT Master */
    init_master(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize torque parameters */
    initialize_torque(slave_number, slv_handles);

    /* Initialize all connected nodes with Mandatory Motor Configurations (specified in motor_config)*/
    init_nodes(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize the node specified with slave_number with CSP configurations (specified under motor_config)*/
    set_operation_mode(CSP, slave_number, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Enable operation of node in CSP mode */
    enable_operation(slave_number, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize position profile parameters */
    initialize_position_profile_limits(slave_number, slv_handles);


    i = 0;
    while(1)
    {
        /* Update the process data (EtherCat packets) sent/received from the node */
        pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

        if(master_setup.op_flag)    /*Check if the master is active*/
        {
            /* Read Actual Position from the node for initialization */
            if(flag == 0)
            {
                 actual_position = get_position_actual_ticks(slave_number, slv_handles);
                 i = i+1;
                 if(i>3)
                 {
                     /* Compute a target position */
                     target_position =  actual_position + 12288;//

                     /* Compute steps needed for the target position */
                     steps = init_position_profile_params(target_position, actual_position, velocity, acceleration, 
                                deceleration, slave_number, slv_handles);
                     flag = 1;
                     i = 1;
                     printf("steps %d target %d actual %d\n", steps, target_position, actual_position);
                 }
            }

            if(i<steps && flag == 1)
            {
                /* Generate target position steps */
                position_ramp =  generate_profile_position(i, slave_number, slv_handles);
                //printf(" position_ramp %d\n", position_ramp);
                /* Send target position for the node specified by slave_number */
                set_position_ticks(position_ramp, slave_number, slv_handles);
                i = i+1;
            }
            if(i >= steps && flag == 1)
            {
                break;
            }

            /* Read actual node sensor values */
            actual_position = get_position_actual_ticks(slave_number, slv_handles);
            actual_velocity = get_velocity_actual_rpm(slave_number, slv_handles);
            actual_torque = get_torque_actual_mNm(slave_number, slv_handles);
            printf("actual position %d actual velocity %d actual_torque %f\n", actual_position, actual_velocity, actual_torque);
        }
    }

    /* Quick stop position mode (for emergency) */
    quick_stop_position(slave_number, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Regain control of node to continue after quick stop */
    renable_ctrl_quick_stop(CSP, slave_number, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES); //after quick-stop

    set_operation_mode(CSP, slave_number, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    enable_operation(slave_number, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Shutdown node operations */
    shutdown_operation(CSP, slave_number, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    return 0;
}

