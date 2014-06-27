
/**
 * \file main.c
 * \brief Example Master App for Cyclic Synchronous Velocity (on PC)
 * \author Pavan Kanajar <pkanajar@synapticon.com>
 * \author Christian Holl <choll@synapticon.com>
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

/* Reading user-entered values */
int * read_user_input(){

    int number, number_of_items;
    static int targets[TOTAL_NUM_OF_SLAVES];
    printf("%d slaves are defined in the system\n", TOTAL_NUM_OF_SLAVES);

    for (int i = 0; i < TOTAL_NUM_OF_SLAVES; i++){

        printf("enter target velocity for slave %d: \n", i+1);
        number_of_items = scanf("%d", &number);

        if (number_of_items == EOF) {
          /* Handle EOF/Failure */
          printf("Input failure\n");
          break_loop = true;
        } else if (number_of_items == 0) {
          /* Handle no match */
          printf("Please type a number without a space!\n");
          break_loop = true;
        } else {
          targets[i] = number;
        }
    }
    return targets;
}


int main()
{
	int flag = 0;

	int final_target_velocity = 2000;			//rpm
	int acceleration= 100;						//rpm/s
	int deceleration = 100;					    //rpm/s
	int steps[TOTAL_NUM_OF_SLAVES];
	int inc[TOTAL_NUM_OF_SLAVES];
	int target_velocity = 0;					// rpm
	int actual_velocity = 0;					// rpm
	int actual_position;						// ticks
	float actual_torque;						// mNm
	enum {SLAVE_1, SLAVE_2};
	int *p;

	p = read_user_input();

	printf("\n*****************************\nLoading motor configurations\n*****************************\n");

	/* Initialize Ethercat Master */
	init_master(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	/* Initialize all connected nodes with Mandatory Motor Configurations (specified under config/motor/)*/
	init_nodes(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	/* Initialize the node specified with slave_number with CSV configurations (specified under config/motor/)*/
	set_operation_mode(CSV, SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	set_operation_mode(CSV, SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	/* Enable operation of node in CSV mode */
	enable_operation(SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	enable_operation(SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Compute steps needed to execute velocity profile */
    for (int i = 0; i < TOTAL_NUM_OF_SLAVES; i++ )
    {
        printf("slave %d SP:  %d\n", i+1, *(p + i));
        final_target_velocity = *(p + i);
        steps[i] = init_velocity_profile_params(final_target_velocity, actual_velocity, acceleration, deceleration, i, slv_handles);
        inc[i] = 1;
    }

	while(1)
	{
		/* Update the process data (EtherCat packets) sent/received from the node */
		pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	    /* catch interrupt signal */
	    signal(SIGINT, INThandler);

		if(master_setup.op_flag && !break_loop) /*Check if the master is active*/
		{
			/* Handling Slave 1 */
		    if(inc[SLAVE_1] < steps[SLAVE_1])
			{
				/* Generate target velocity steps */
				target_velocity = generate_profile_velocity( inc[SLAVE_1], SLAVE_1, slv_handles);

				/* Send target velocity for the node specified by slave_number */
				set_velocity_rpm(target_velocity, SLAVE_1, slv_handles);

				/* Read actual node sensor values */
				actual_velocity = get_velocity_actual_rpm(SLAVE_1, slv_handles);
				actual_position = get_position_actual_ticks(SLAVE_1, slv_handles);
				actual_torque = get_torque_actual_mNm(SLAVE_1, slv_handles);
				printf("\nVelocity slave 1: %d Position slave 1: %d Torque slave 1: %4.2f\n",
				        actual_velocity, actual_position, actual_torque);
				inc[SLAVE_1]++;
			}

		    /* Handling Slave 2 */
            if(inc[SLAVE_2] < steps[SLAVE_2])
            {
                /* Generate target velocity steps */
                target_velocity = generate_profile_velocity( inc[SLAVE_2], SLAVE_2, slv_handles);

                /* Send target velocity for the node specified by slave_number */
                set_velocity_rpm(target_velocity, SLAVE_2, slv_handles);

                /* Read actual node sensor values */
                actual_velocity = get_velocity_actual_rpm(SLAVE_2, slv_handles);
                actual_position = get_position_actual_ticks(SLAVE_2, slv_handles);
                actual_torque = get_torque_actual_mNm(SLAVE_2, slv_handles);
                printf("Velocity slave 2: %d Position slave 2: %d Torque slave 2: %4.2f\n",
                        actual_velocity, actual_position, actual_torque);
                inc[SLAVE_2]++;
            }


            if(inc[SLAVE_1] >= steps[SLAVE_1] && inc[SLAVE_2] >= steps[SLAVE_2])
			{
                printf("Target velocities are reached. Press 'Ctrl + C' to quit! |  ");
                printf("slave 1: %4d   slave 2: %4d     \r",
                        get_velocity_actual_rpm(SLAVE_1, slv_handles),
                        get_velocity_actual_rpm(SLAVE_2, slv_handles));
			}
		}
		else if (break_loop){
            break;
        }
	}

	/* Quick stop velocity mode (for emergency) */
	quick_stop_velocity(SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	quick_stop_velocity(SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	/* Regain control of node to continue after quick stop */
	renable_ctrl_quick_stop(CSV, SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	renable_ctrl_quick_stop(CSV, SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	set_operation_mode(CSV, SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	set_operation_mode(CSV, SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	enable_operation(SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	enable_operation(SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	/* Shutdown node operations */
	shutdown_operation(CSV, SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	shutdown_operation(CSV, SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	return 0;
}


