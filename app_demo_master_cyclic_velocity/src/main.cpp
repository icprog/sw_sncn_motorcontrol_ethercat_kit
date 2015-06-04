/**
 * @file main.c
 * @brief Example Master App for Cyclic Synchronous Velocity (on PC)
 * @author Synapticon GmbH (www.synapticon.com)
 */


#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#include "CEtherCATdrive.h"
#include "ethercat_setup.h"


/* Only here for interrupt signaling */
bool break_loop = false;

/* Interrupt signal handler */

void  INThandler(int sig)
{
     signal(sig, SIG_IGN);
     break_loop = true;
     signal(SIGINT, INThandler);
}


int main() {

    int target_velocity = 2000; //rpm
    int acceleration = 200; //rpm/s
    int deceleration = 200; //rpm/s

    int actual_velocity = 0; // rpm
    int actual_position; // ticks
    float actual_torque; // mNm
    int steps = 0;
    int velocity_ramp = 0; // rpm


    C_EtherCAT_drive o_EtherCAT;

    o_EtherCAT.init_master_(o_EtherCAT.ECAT_SLAVE_0, TOTAL_NUM_OF_SLAVES, &master_setup, slv_handles);

    /* Initialize the node specified with ECAT_SLAVE_0 with CSV configurations (specified in config/motor/)*/
    o_EtherCAT.set_operation_mode_(CSV, o_EtherCAT.ECAT_SLAVE_0, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Enable operation of node in CSV mode */
    o_EtherCAT.enable_operation_(o_EtherCAT.ECAT_SLAVE_0, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    steps = o_EtherCAT.calculate_profile_steps_(target_velocity, actual_velocity, acceleration, deceleration, o_EtherCAT.ECAT_SLAVE_0, slv_handles);


    /* catch interrupt signal */
    signal(SIGINT, INThandler);

    /* Just for better printing result */
    printf("\n");
    system("setterm -cursor off");

    while(1)
    {
        if (master_setup.op_flag && actual_velocity == 0) /*Check if the master is active and we haven't started moving yet*/
        {
            for (int step = 1; step < steps + 1; step++) {

                /* Update the process data (EtherCat packets) sent/received from the node */
                o_EtherCAT.update_PDOs(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

                /* Generate target velocity steps */
                velocity_ramp = o_EtherCAT.calculate_next_setpoint(step, o_EtherCAT.ECAT_SLAVE_0, slv_handles);

                /* Send target velocity for the node specified by ECAT_SLAVE_0 */
                o_EtherCAT.set_velocity(velocity_ramp, o_EtherCAT.ECAT_SLAVE_0, slv_handles);

                /* Read actual node sensor values */
                actual_velocity = o_EtherCAT.get_velocity(o_EtherCAT.ECAT_SLAVE_0, slv_handles);
                actual_position = o_EtherCAT.get_position(o_EtherCAT.ECAT_SLAVE_0, slv_handles);
                actual_torque = o_EtherCAT.get_torque(o_EtherCAT.ECAT_SLAVE_0, slv_handles);

                printf("\r    Velocity: %d    Position: %d    Torque: %f        ",
                        actual_velocity, actual_position, actual_torque);
            }
        }
        else if (break_loop){
            break;
        }
        else {
            /* Update the process data (EtherCat packets) sent/received from the node */
            pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
            /* Read actual node sensor values */
            actual_velocity = o_EtherCAT.get_velocity(o_EtherCAT.ECAT_SLAVE_0, slv_handles);
            actual_position = o_EtherCAT.get_position(o_EtherCAT.ECAT_SLAVE_0, slv_handles);
            actual_torque = o_EtherCAT.get_torque(o_EtherCAT.ECAT_SLAVE_0, slv_handles);

            printf("\r    Velocity: %d    Position: %d    Torque: %f        ",
                    actual_velocity, actual_position, actual_torque);
        }
    }
    printf("\n");

    /* Quick stop velocity mode (for emergency) */
    o_EtherCAT.quick_stop_node(CSV, o_EtherCAT.ECAT_SLAVE_0, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Shutdown node operations */
    o_EtherCAT.shutdown_operation_(CSV, o_EtherCAT.ECAT_SLAVE_0, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Just for better printing result */
    system("setterm -cursor on");

    return 0;
}

