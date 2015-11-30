/* INCLUDE BOARD SUPPORT FILES FROM module_board-support */
#include <CORE_C22-rev-a.inc>
#include <IFM_DC100-rev-b.inc>

/**
 * @file test_torque-ctrl.xc
 * @brief Test illustrates usage of profile torque control
 * @author Synapticon GmbH (www.synapticon.com)
 */

#include <print.h>
#include <hall_server.h>
#include <qei_server.h>
#include <biss_server.h>
#include <pwm_service_inv.h>
#include <adc_server_ad7949.h>
#include <commutation_server.h>
#include <refclk.h>
#include <xscope.h>
#include <torque_ctrl_server.h>
#include <profile_control.h>
#include <drive_modes.h>
#include <statemachine.h>
#include <torque_ctrl_client.h>
#include <profile.h>
#include <internal_config.h>
//Configure your motor parameters in config/bldc_motor_config.h
#include <bldc_motor_config.h>


on tile[IFM_TILE]: clock clk_adc = XS1_CLKBLK_1;
on tile[IFM_TILE]: clock clk_pwm = XS1_CLKBLK_REF;
on tile[IFM_TILE]: clock clk_biss = XS1_CLKBLK_2 ;
port out p_ifm_biss_clk = GPIO_D0;


/* Test Profile Torque Function */
void profile_torque_test(chanend c_torque_ctrl)
{
	int target_torque = 500; 	//(desired torque/torque_constant)  * IFM resolution
	int torque_slope  = 30;  	//(desired torque_slope/torque_constant)  * IFM resolution
	cst_par cst_params; int actual_torque; timer t; unsigned int time;
	init_cst_param(cst_params);
    xscope_int(TARGET_TORQUE, target_torque);

	/* Set new target torque for profile torque control */
	set_profile_torque( target_torque, torque_slope, cst_params, c_torque_ctrl);

	target_torque = 0;
	xscope_int(TARGET_TORQUE, target_torque);
	set_profile_torque( target_torque, torque_slope, cst_params, c_torque_ctrl);

	target_torque = -200;
	xscope_int(TARGET_TORQUE, target_torque);
	set_profile_torque( target_torque, torque_slope, cst_params, c_torque_ctrl);
	t:>time;
	while(1)
	{
		actual_torque = get_torque(c_torque_ctrl)*cst_params.polarity;

        xscope_int(ACTUAL_TORQUE, actual_torque);
        xscope_int(TARGET_TORQUE, target_torque);

		t when timerafter(time + MSEC_STD) :> time;
	}
}

int main(void)
{
	// Motor control channels
	chan c_adc, c_adctrig;													// adc channels
	chan c_qei_p1, c_qei_p2, c_qei_p3, c_qei_p4, c_qei_p5, c_qei_p6 ; 		// qei channels
	chan c_hall_p1, c_hall_p2, c_hall_p3, c_hall_p4, c_hall_p5, c_hall_p6;	// hall channels
	chan c_commutation_p1, c_signal;	                                    // commutation channels
	chan c_pwm_ctrl;														// pwm channel
	chan c_torque_ctrl,c_velocity_ctrl, c_position_ctrl;					// torque control channel
	chan c_watchdog; 														// watchdog channel
	interface i_biss i_biss[2];                                             // biss interfaces

	par
	{

		/* Test Profile Torque Function */
		on tile[0]:
		{
			profile_torque_test(c_torque_ctrl);
			//torque_ctrl_unit_test(c_torque_ctrl, c_qei_p4, c_hall_p4);
		}

		on tile[2]:
		{
			par
			{
				/* Torque Control Loop */
				{
					ctrl_par torque_ctrl_params;
					hall_par hall_params;
					qei_par qei_params;

					/* Initialize PID parameters for Torque Control (defined in config/motor/bldc_motor_config.h) */
					init_torque_control_param(torque_ctrl_params);

					/* Initialize Sensor configuration parameters (defined in config/motor/bldc_motor_config.h) */
					init_qei_param(qei_params);
					init_hall_param(hall_params);

					/* Control Loop */
					torque_control( torque_ctrl_params, hall_params, qei_params, SENSOR_USED,
							c_adc, c_commutation_p1,  c_hall_p3,  c_qei_p3, i_biss[1], c_torque_ctrl);
				}
			}
		}

		/************************************************************
		 * IFM_TILE
		 ************************************************************/
		on tile[IFM_TILE]:
		{
			par
			{
				/* ADC Loop */
				adc_ad7949_triggered(c_adc, c_adctrig, clk_adc,\
						p_ifm_adc_sclk_conv_mosib_mosia, p_ifm_adc_misoa,\
						p_ifm_adc_misob);

				/* PWM Loop */
				do_pwm_inv_triggered(c_pwm_ctrl, c_adctrig, p_ifm_dummy_port,
						p_ifm_motor_hi, p_ifm_motor_lo, clk_pwm);

				/* Motor Commutation loop */
				{
					hall_par hall_params;
					qei_par qei_params;
					commutation_par commutation_params;
					init_hall_param(hall_params);
					init_qei_param(qei_params);
					commutation_sinusoidal(c_hall_p1,  c_qei_p1, i_biss[0], c_signal, c_watchdog, 	\
							c_commutation_p1, null, null, c_pwm_ctrl,\
							p_ifm_esf_rstn_pwml_pwmh, p_ifm_coastn, p_ifm_ff1, p_ifm_ff2,\
							hall_params, qei_params, commutation_params, HALL);
				}

				/* Watchdog Server */
				run_watchdog(c_watchdog, p_ifm_wd_tick, p_ifm_shared_leds_wden);

				/* Hall Server */
				{
					hall_par hall_params;
					run_hall(c_hall_p1, c_hall_p2, c_hall_p3, c_hall_p4, c_hall_p5, c_hall_p6, p_ifm_hall, hall_params); // channel priority 1,2..4
				}
#if (SENSOR_USED != BISS)
				/* QEI Server */
				{
					qei_par qei_params;
					init_qei_param(qei_params);
					run_qei(c_qei_p1, c_qei_p2, c_qei_p3, c_qei_p4, c_qei_p5, c_qei_p6, p_ifm_encoder, qei_params);  // channel priority 1,2..4
				}
#else
				/* biss server */
				{
				    biss_par biss_params;
				    run_biss(i_biss, 2, p_ifm_biss_clk, p_ifm_encoder, clk_biss, biss_params, 2);
				}
#endif
			}
		}

	}

	return 0;
}
