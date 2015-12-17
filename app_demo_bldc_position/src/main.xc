/* INCLUDE BOARD SUPPORT FILES FROM module_board-support */
#include <CORE_C22-rev-a.inc>
//#include <IFM_DC100-rev-b.inc>
#include <IFM_DC1K-rev-c1.inc>

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
#include <xscope.h>
#include <qei_server.h>
#include <profile.h>
#include <position_ctrl_server.h>
#include <drive_modes.h>
#include <statemachine.h>
#include <profile_control.h>
#include <drive_modes.h>
#include <position_ctrl_client.h>
#include <internal_config.h>
#include <biss_server.h>
//Configure your motor parameters in config/bldc_motor_config.h
#include <bldc_motor_config.h>



on tile[IFM_TILE]: clock clk_adc = XS1_CLKBLK_1;
on tile[IFM_TILE]: clock clk_pwm = XS1_CLKBLK_REF;
on tile[IFM_TILE]: clock clk_biss = XS1_CLKBLK_2 ;
port out p_ifm_biss_clk = GPIO_D0;

void pwm_output(buffered out port:32 p_pwm, buffered out port:32 p_pwm_inv, int duty, int period, int msec) {
    const unsigned delay = 5*USEC_FAST;
    timer t;
    unsigned int ts;
    if (msec) {
        t :> ts;
        msec = ts + msec*MSEC_FAST;
    }

    while(1) {
        p_pwm <: 0xffffffff;
        delay_ticks(period*duty);
        p_pwm <: 0x00000000;
        delay_ticks(delay);
        p_pwm_inv<: 0xffffffff;
        delay_ticks(period*(100-duty) + 2*delay);
        p_pwm_inv <: 0x00000000;
        delay_ticks(delay);

        if (msec) {
            t :> ts;
            if (timeafter(ts, msec))
                break;
        }
    }
}
void brake_release(buffered out port:32 p_pwm,  buffered out port:32 p_pwm_inv) {
    printf("*************************************\n        BRAKE RELEASE\n*************************************\n");
    p_pwm <: 0;
    p_pwm_inv <: 0;
    pwm_output(p_pwm, p_pwm_inv, 100, 100, 100);
    pwm_output(p_pwm, p_pwm_inv, 22, 10, 0);
}

/* Test Profile Position function */
void position_profile_test(chanend c_position_ctrl)
{
	int actual_position = 0;			// ticks
	int target_position = ENCODER_RESOLUTION*3;		// HALL: 4096 extrapolated ticks x nr. pole pairs = one rotation; QEI: your encoder documented resolution x 4 = one rotation
	int velocity 		= 700;			// rpm
	int acceleration 	= 100;			// rpm/s
	int deceleration 	= 100;     	    // rpm/s
	int follow_error;
	timer t;
	hall_par hall_params;
	qei_par qei_params;
	init_qei_param(qei_params);
	init_hall_param(hall_params);

	/* Initialise Profile Limits for position profile generator and select position sensor */
	init_position_profile_limits(MAX_ACCELERATION, MAX_PROFILE_VELOCITY, qei_params, hall_params, \
			SENSOR_USED, MAX_POSITION_LIMIT, MIN_POSITION_LIMIT);

	actual_position = get_position(c_position_ctrl);
	target_position += actual_position;

	/* Set new target position for profile position control */
	set_profile_position(target_position, velocity, acceleration, deceleration, SENSOR_USED, c_position_ctrl);

	while(1)
	{
	    /* Read actual position from the Position Control Server */
		actual_position = get_position(c_position_ctrl);
		follow_error = target_position - actual_position;

		xscope_int(ACTUAL_POSITION, actual_position);
		xscope_int(TARGET_POSITION, target_position);
		xscope_int(FOLLOW_ERROR, follow_error);

		wait_ms(1, 1, t);  /* 1 ms wait */
	}
}


int main(void)
{
	// Motor control channels
	chan c_qei_p1, c_qei_p2, c_qei_p5;		    // qei channels
	chan c_hall_p1, c_hall_p2, c_hall_p5;	    // hall channels
	chan c_commutation_p3;	                    // commutation channels
	chan c_pwm_ctrl, c_adctrig;				    // pwm channels
	chan c_position_ctrl;					    // position control channel
	chan c_watchdog; 						    // watchdog channel
	interface i_biss i_biss[2];                 // biss interfaces

	par
	{
		/* Test Profile Position Client function*/
		on tile[APP_TILE_1]:
		{
			position_profile_test(c_position_ctrl);		// test PPM on slave side
			//position_ctrl_unit_test(c_position_ctrl, c_qei_p5, c_hall_p5); 	// Unit test controller
		}


		on tile[APP_TILE_1]:
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
						 c_qei_p2, i_biss[1], c_position_ctrl, c_commutation_p3);
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
                    disable_fets(p_ifm_motor_hi, p_ifm_motor_lo, 3);
#endif
                    do_pwm_inv_triggered(c_pwm_ctrl, c_adctrig, p_ifm_dummy_port, p_ifm_motor_hi, p_ifm_motor_lo, clk_pwm);
                }

                /* Brake release */
                brake_release(p_ifm_motor_hi_d, p_ifm_motor_lo_d);

				/* Motor Commutation loop */
				{
					hall_par hall_params;
					qei_par qei_params;
					commutation_par commutation_params;
					init_hall_param(hall_params);
					init_qei_param(qei_params);
					commutation_sinusoidal(c_hall_p1,  c_qei_p1, i_biss[0], null, c_watchdog,
							null, null, c_commutation_p3, c_pwm_ctrl,
#ifdef DC1K
                            null, null, null, null,
#else
							p_ifm_esf_rstn_pwml_pwmh, p_ifm_coastn, p_ifm_ff1, p_ifm_ff2,
#endif
							hall_params, qei_params, commutation_params, BISS);
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
                    p_ifm_encoder_hall_select_ext_d4to5 <: SET_PORT1_AS_QEI_PORT2_AS_HALL;//last two bits define the interface [con2, con1], 0 - hall, 1 - QEI.
#endif
                    run_hall(c_hall_p1, c_hall_p2, null, null, c_hall_p5,null, p_ifm_hall, hall_params); // channel priority 1,2..6
				}

#if (SENSOR_USED != BISS)
				/* QEI Server */
				{
					qei_par qei_params;

					//connector 2 is configured as QEI
                    run_qei(c_qei_p1, c_qei_p2, null, null, c_qei_p5, null, p_ifm_encoder, qei_params);          // channel priority 1,2..5
				}
#else
				/* biss server */
				{
				    biss_par biss_params;
				    run_biss(i_biss, 2, p_ifm_biss_clk, p_ifm_encoder, clk_biss, biss_params, BISS_FRAME_BYTES);
				}
#endif
			}
		}

	}

	return 0;
}
