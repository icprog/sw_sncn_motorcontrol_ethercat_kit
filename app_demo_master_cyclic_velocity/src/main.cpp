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

#include <iostream>
#include <cstdlib>
#include <pthread.h>
using namespace std;

#define NUM_THREADS     1

/* Only here for interrupt signaling */
bool break_loop = false;

struct control_values{
   int thread_id;
   int target_velocity; //rpm
   int acceleration; //rpm/s
   int deceleration; //rpm/s

   int actual_velocity; // rpm
   int actual_position; // ticks
   float actual_torque; // mNm
   int steps;
   int velocity_ramp; // rpm
};

/* Interrupt signal handler */

void  INThandler(int sig)
{
     signal(sig, SIG_IGN);
     break_loop = true;
     signal(SIGINT, INThandler);
}

void *run_velocity_control(void *threadarg)
{
    struct control_values *my_control_values;
    my_control_values = (struct control_values *) threadarg;

    cout << "Thread ID : " << my_control_values->thread_id << endl;


    C_EtherCAT_drive o_EtherCAT;

    o_EtherCAT.init_master_(o_EtherCAT.ECAT_SLAVE_0, TOTAL_NUM_OF_SLAVES, &master_setup, slv_handles);

    /* Initialize the node specified with ECAT_SLAVE_0 with CSV configurations (specified in config/motor/)*/
    o_EtherCAT.set_operation_mode_(CSV, o_EtherCAT.ECAT_SLAVE_0, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Enable operation of node in CSV mode */
    o_EtherCAT.enable_operation_(o_EtherCAT.ECAT_SLAVE_0, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    my_control_values->steps = o_EtherCAT.calculate_profile_steps_(my_control_values->target_velocity, my_control_values->actual_velocity,
                                my_control_values->acceleration, my_control_values->deceleration, o_EtherCAT.ECAT_SLAVE_0, slv_handles);


    /* catch interrupt signal */
    signal(SIGINT, INThandler);

    /* Just for better printing result */
    printf("\n");
    system("setterm -cursor off");

    while(1)
    {
        if (master_setup.op_flag && my_control_values->actual_velocity == 0) /*Check if the master is active and we haven't started moving yet*/
        {
            for (int step = 1; step < my_control_values->steps + 1; step++) {

                /* Update the process data (EtherCat packets) sent/received from the node */
                o_EtherCAT.update_PDOs(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

                /* Generate target velocity steps */
                my_control_values->velocity_ramp = o_EtherCAT.calculate_next_setpoint(step, o_EtherCAT.ECAT_SLAVE_0, slv_handles);

                /* Send target velocity for the node specified by ECAT_SLAVE_0 */
                o_EtherCAT.set_velocity(my_control_values->velocity_ramp, o_EtherCAT.ECAT_SLAVE_0, slv_handles);

                /* Read actual node sensor values */
                my_control_values->actual_velocity = o_EtherCAT.get_velocity(o_EtherCAT.ECAT_SLAVE_0, slv_handles);
                my_control_values->actual_position = o_EtherCAT.get_position(o_EtherCAT.ECAT_SLAVE_0, slv_handles);
                my_control_values->actual_torque = o_EtherCAT.get_torque(o_EtherCAT.ECAT_SLAVE_0, slv_handles);

                printf("\r    Velocity: %d    Position: %d    Torque: %f        ",
                        my_control_values->actual_velocity, my_control_values->actual_position, my_control_values->actual_torque);
            }
        }
        else if (break_loop){
            break;
        }
        else {
            /* Update the process data (EtherCat packets) sent/received from the node */
            pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
            /* Read actual node sensor values */
            my_control_values->actual_velocity = o_EtherCAT.get_velocity(o_EtherCAT.ECAT_SLAVE_0, slv_handles);
            my_control_values->actual_position = o_EtherCAT.get_position(o_EtherCAT.ECAT_SLAVE_0, slv_handles);
            my_control_values->actual_torque = o_EtherCAT.get_torque(o_EtherCAT.ECAT_SLAVE_0, slv_handles);

            printf("\r    Velocity: %d    Position: %d    Torque: %f        ",
                    my_control_values->actual_velocity, my_control_values->actual_position, my_control_values->actual_torque);
        }

    }
    printf("\n");

    /* Quick stop velocity mode (for emergency) */
    o_EtherCAT.quick_stop_node(CSV, o_EtherCAT.ECAT_SLAVE_0, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Shutdown node operations */
    o_EtherCAT.shutdown_operation_(CSV, o_EtherCAT.ECAT_SLAVE_0, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Just for better printing result */
    system("setterm -cursor on");

    pthread_exit(NULL);
}


int main() {

    pthread_t threads[NUM_THREADS];
    struct control_values task_data;
    int rc;

    cout <<"main() : creating thread, " << endl;
    task_data.thread_id = 42;
    task_data.target_velocity = 2000; //rpm
    task_data.acceleration = 200; //rpm/s
    task_data.deceleration = 200; //rpm/s

    task_data.actual_velocity = 0; // rpm
    task_data.actual_position = 0; // ticks
    task_data.actual_torque = 0; // mNm
    task_data.steps = 0;
    task_data.velocity_ramp = 0; // rpm


    rc = pthread_create(&threads[0], NULL, run_velocity_control, (void *)&task_data);
    if (rc){
     cout << "Error:unable to create thread," << rc << endl;
     exit(-1);
    }

    pthread_exit(NULL);

    return 0;
}

