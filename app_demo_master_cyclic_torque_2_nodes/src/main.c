
/**
 * @file main.c
 * @brief Example Master App for Cyclic Synchronous Torque (on PC)
 * @author Pavan Kanajar <pkanajar@synapticon.com>
 * @author Christian Holl <choll@synapticon.com>
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
#include <unistd.h>
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

        printf("enter target torque for slave %d: \n", i+1);
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
	float final_target_torque = 0.0;			// mNm
	float torque_slope = 10.0;					// mNm/s
	int steps[TOTAL_NUM_OF_SLAVES];
	int inc_slave_1 = 1;
	int inc_slave_2 = 1;

	float target_torque_slave_1 = 0.0;  		// mNm
    float target_torque_slave_2 = 0.0;          // mNm

	float actual_torque_slave_1 = 0.0;		    // mNm
	float actual_torque_slave_2 = 0.0;          // mNm

	int actual_position_slave_1 = 0;			// ticks
	int actual_velocity_slave_1 = 0;			// rpm

    int actual_position_slave_2 = 0;            // ticks
    int actual_velocity_slave_2 = 0;            // rpm

    #define MAX_VELOCITY 4000

    bool new_target_slave_1 = true;
    bool new_target_slave_2 = true;

	enum {SLAVE_1, SLAVE_2};
    int *p;

    p = read_user_input();

    printf("\n*****************************\nLoading motor configurations\n*****************************\n");

    /* Initialize Ethercat Master */
	init_master(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* catch interrupt signal */
    signal(SIGINT, INThandler);

    /* Initialize all connected nodes with Mandatory Motor Configurations (specified under config/motor/)*/
	init_nodes(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	/* Initialize the node specified with slave_number with CST configurations (specified under config/motor/)*/
	set_operation_mode(CST, SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	set_operation_mode(CST, SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	/* Enable operation of node in CST mode */
	enable_operation(SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	enable_operation(SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Compute steps needed for the target torque */
    for (int i = 0; i < TOTAL_NUM_OF_SLAVES; i++ )
    {
        printf("slave %d SP:  %d\n", i+1, *(p + i));
        final_target_torque = *(p + i);
        steps[i] = init_linear_profile_params(final_target_torque, actual_torque_slave_1, torque_slope, i, slv_handles);
    }


	while(1)
	{
		/* Update the process data (EtherCat packets) sent/received from the node */
		pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

		if(master_setup.op_flag && !break_loop) /*Check if the master is active*/
		{
			if(inc_slave_1 < steps[SLAVE_1])
			{
				/* Generate target torque steps */
				target_torque_slave_1 = generate_profile_linear(inc_slave_1, SLAVE_1, slv_handles);
				printf("\ntarget_torque slave 1: %f \n",target_torque_slave_1);
				/* Send target torque for the node specified by slave_number */
				set_torque_mNm(target_torque_slave_1, SLAVE_1, slv_handles);

				/* Read actual node sensor values */
				actual_torque_slave_1 = get_torque_actual_mNm(SLAVE_1, slv_handles);
				actual_position_slave_1 = get_position_actual_ticks(SLAVE_1, slv_handles);
				actual_velocity_slave_1 = get_velocity_actual_rpm(SLAVE_1, slv_handles);
				printf("actual_torque slave 1: %f actual_position slave 1: %d actual_velocity slave 1: %d\n",
				        actual_torque_slave_1, actual_position_slave_1, actual_velocity_slave_1);

                /* Trigger emergency stop if max velocity is reached*/
                if (actual_velocity_slave_1 > MAX_VELOCITY){
                    printf("\nDanger! Max velocity is reached\n");
                    break_loop = true;
                }

				inc_slave_1++;
			}

			if(inc_slave_2 < steps[SLAVE_2])
            {
                /* Generate target torque steps */
                target_torque_slave_2 = generate_profile_linear(inc_slave_2, SLAVE_2, slv_handles);
                printf("\ntarget_torque slave 2: %f \n",target_torque_slave_2);
                /* Send target torque for the node specified by slave_number */
                set_torque_mNm(target_torque_slave_2, SLAVE_2, slv_handles);

                /* Read actual node sensor values */
                actual_torque_slave_2 = get_torque_actual_mNm(SLAVE_2, slv_handles);
                actual_position_slave_2 = get_position_actual_ticks(SLAVE_2, slv_handles);
                actual_velocity_slave_2 = get_velocity_actual_rpm(SLAVE_2, slv_handles);
                printf("actual_torque slave 2: %f actual_position slave 2: %d actual_velocity slave 2: %d\n",
                        actual_torque_slave_2, actual_position_slave_2, actual_velocity_slave_2);

                /* Trigger emergency stop if max velocity is reached*/
                if (actual_velocity_slave_2 > MAX_VELOCITY){
                    printf("\nDanger! Max velocity is reached\n");
                    break_loop = true;
                }

                inc_slave_2++;
            }

			if(inc_slave_1 >= steps[SLAVE_1] && inc_slave_2 >= steps[SLAVE_2])
			{
			    actual_torque_slave_1 = get_torque_actual_mNm(SLAVE_1, slv_handles);
			    actual_torque_slave_2 = get_torque_actual_mNm(SLAVE_2, slv_handles);
				printf("Target torques are reached. ");
				printf("slave 1: %4.2f     slave 2: %4.2f     \r", actual_torque_slave_1, actual_torque_slave_2);
			}
		}
		else if (break_loop){
		    break;
		}
	}



	/* Quick stop torque mode (for emergency) */
	quick_stop_torque(SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	quick_stop_torque(SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	/* Regain control of node to continue after quick stop */
	renable_ctrl_quick_stop(CST, SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	renable_ctrl_quick_stop(CST, SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	set_operation_mode(CST, SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	set_operation_mode(CST, SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	enable_operation(SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	enable_operation(SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	/* Shutdown node operations */
	shutdown_operation(CST, SLAVE_1, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
	shutdown_operation(CST, SLAVE_2, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

	return 0;
}


