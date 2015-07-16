/* INCLUDE BOARD SUPPORT FILES FROM module_board-support */
#include <CORE_C22-rev-a.inc>
#include <IFM_DC100-rev-b.inc>

/**
 * @file test_position-ctrl.xc
 * @brief Test illustrates usage of profile position control
 * @author Synapticon GmbH (www.synapticon.com)
 */

#include <print.h>
#include <hall_server.h>
#include <pwm_service_inv.h>
#include <commutation_server.h>
#include <refclk.h>
#include <xscope_wrapper.h>
#include <qei_server.h>
#include <profile.h>
#include <position_ctrl_server.h>
#include <drive_modes.h>
#include <statemachine.h>
#include <profile_control.h>
#include <drive_modes.h>
#include <position_ctrl_client.h>
#include <internal_config.h>
//Configure your motor parameters in config/bldc_motor_config.h
#include <bldc_motor_config.h>


//#define ENABLE_xscope

on tile[IFM_TILE]: clock clk_adc = XS1_CLKBLK_1;
on tile[IFM_TILE]: clock clk_pwm = XS1_CLKBLK_REF;


void xscope_initialise_1()
{
	xscope_register(3,
	        XSCOPE_CONTINUOUS, "0 actual_position", XSCOPE_INT,	"n",
			XSCOPE_CONTINUOUS, "1 target_position", XSCOPE_INT, "n",
			XSCOPE_CONTINUOUS, "2 follow_error"   , XSCOPE_INT, "n");
}


/* Test Profile Position function */
void position_profile_test(chanend c_position_ctrl, chanend c_qei, chanend c_hall)
{
	int actual_position = 0;			// ticks
	int target_position = 4096*3;		// HALL: 4096 extrapolated ticks x nr. pole pairs = one rotation; QEI: your encoder documented resolution x 4 = one rotation
	int velocity 		= 100;			// rpm
	int acceleration 	= 100;			// rpm/s
	int deceleration 	= 100;     	// rpm/s
	int follow_error;
	timer t;
	hall_par hall_params;
	qei_par qei_params;
	init_qei_param(qei_params);
	init_hall_param(hall_params);

	/* Initialise Profile Limits for position profile generator and select position sensor */
	init_position_profile_limits(MAX_ACCELERATION, MAX_PROFILE_VELOCITY, qei_params, hall_params, \
			SENSOR_USED, MAX_POSITION_LIMIT, MIN_POSITION_LIMIT);

#ifdef ENABLE_xscope
	xscope_initialise_1();
#endif

	/* Set new target position for profile position control */
	set_profile_position(target_position, velocity, acceleration, deceleration, SENSOR_USED, c_position_ctrl);

	/* Read actual position from the Position Control Server */
	actual_position = get_position(c_position_ctrl);

	while(1)
	{
		actual_position = get_position(c_position_ctrl);
		follow_error = target_position - actual_position;
#ifdef ENABLE_xscope
		xscope_int(0, actual_position);
		xscope_int(1, target_position);
		xscope_int(2, follow_error);
#endif
		wait_ms(1, 1, t);  /* 1 ms wait */
	}
}

int main(void)
{
	// Motor control channels
	chan c_qei_p1, c_qei_p2, c_qei_p3, c_qei_p4, c_qei_p5, c_qei_p6;		// qei channels
	chan c_hall_p1, c_hall_p2, c_hall_p3, c_hall_p4, c_hall_p5, c_hall_p6;	// hall channels
	chan c_commutation_p1, c_commutation_p2, c_commutation_p3, c_signal;	// commutation channels
	chan c_pwm_ctrl, c_adctrig;												// pwm channels
	chan c_position_ctrl;													// position control channel
	chan c_watchdog; 														// watchdog channel

	par
	{
		/* Test Profile Position Client function*/
		on tile[APP_TILE]:
		{
			position_profile_test(c_position_ctrl, c_qei_p5, c_hall_p5);		// test PPM on slave side
			//position_ctrl_unit_test(c_position_ctrl, c_qei_p5, c_hall_p5); 	// Unit test controller
		}


		on tile[APP_TILE]:
		{
			/* Position Control Loop */
			{
				 ctrl_par position_ctrl_params;
				 hall_par hall_params;
				 qei_par qei_params;

				 /* Initialize PID parameters for Position Control (defined in config/motor/bldc_motor_config.h) */
				 init_position_control_param(position_ctrl_params);

				 /* Initialize Sensor configuration parameters (defined in config/motor/bldc_motor_config.h) */
				 init_hall_param(hall_params);
				 init_qei_param(qei_params);

				 /* Control Loop */
				 position_control(position_ctrl_params, hall_params, qei_params, SENSOR_USED, c_hall_p2,\
						 c_qei_p2, c_position_ctrl, c_commutation_p3);
			}

		}

		/************************************************************
		 * IFM_TILE
		 ************************************************************/
		on tile[IFM_TILE]:
		{
			par
			{
				/* PWM Loop */
                {
#ifdef DC1K

                    // Turning off all MOSFETs for for initialization
                    p_ifm_motor_hi[0] <: 0;
                    p_ifm_motor_hi[1] <: 0;
                    p_ifm_motor_hi[2] <: 0;
                    p_ifm_motor_hi[3] <: 0;
                    p_ifm_motor_lo[0] <: 0;
                    p_ifm_motor_lo[1] <: 0;
                    p_ifm_motor_lo[2] <: 0;
                    p_ifm_motor_lo[3] <: 0;

                    delay_milliseconds(1);
#endif
                    do_pwm_inv_triggered(c_pwm_ctrl, c_adctrig, p_ifm_dummy_port, p_ifm_motor_hi, p_ifm_motor_lo, clk_pwm);
                }

				/* Motor Commutation loop */
				{
					hall_par hall_params;
					qei_par qei_params;
					commutation_par commutation_params;
					init_hall_param(hall_params);
					init_qei_param(qei_params);
					commutation_sinusoidal(c_hall_p1,  c_qei_p1, c_signal, c_watchdog,
							c_commutation_p1, c_commutation_p2, c_commutation_p3, c_pwm_ctrl,
#ifdef DC1K
                            null, null, null, null,
#else
							p_ifm_esf_rstn_pwml_pwmh, p_ifm_coastn, p_ifm_ff1, p_ifm_ff2,
#endif
							hall_params, qei_params, commutation_params);
				}

				/* Watchdog Server */
#ifdef DC1K
                run_watchdog(c_watchdog, null, p_ifm_led_moton_wdtick_wden);
#else
                run_watchdog(c_watchdog, p_ifm_wd_tick, p_ifm_shared_leds_wden);
#endif

				/* Hall Server */
				{
					hall_par hall_params;
#ifdef DC1K
                    //connector 1 is configured as hall
                    p_ifm_encoder_hall_select_ext_d4to5 <: 0b0010;//last two bits define the interface [con2, con1], 0 - hall, 1 - QEI.
                    run_hall(c_hall_p1, c_hall_p2, c_hall_p3, c_hall_p4, c_hall_p5, c_hall_p6, p_ifm_encoder_hall_1, hall_params); // channel priority 1,2..6

#else
					run_hall(c_hall_p1, c_hall_p2, c_hall_p3, c_hall_p4, c_hall_p5, c_hall_p6, p_ifm_hall, hall_params); // channel priority 1,2..6
#endif
				}

				/* QEI Server */
				{
					qei_par qei_params;
#ifdef DC1K
                    //connector 2 is configured as QEI
                    run_qei(c_qei_p1, c_qei_p2, c_qei_p3, c_qei_p4, c_qei_p5, c_qei_p6, p_ifm_encoder_hall_2, qei_params);          // channel priority 1,2..5
#else
					run_qei(c_qei_p1, c_qei_p2, c_qei_p3, c_qei_p4, c_qei_p5, c_qei_p6, p_ifm_encoder, qei_params);  	// channel priority 1,2..6
#endif
				}
			}
		}

	}

	return 0;
}
