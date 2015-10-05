/* INCLUDE BOARD SUPPORT FILES FROM module_board-support */
#include <CORE_C22-rev-a.inc>
#include <IFM_DC1K-rev-c.inc>

/**
 * @file test_velocity-ctrl.xc
 * @brief Test illustrates usage of profile velocity control
 * @author Synapticon GmbH (www.synapticon.com)
 */

#include <print.h>
#include <hall_server.h>
#include <qei_server.h>
#include <pwm_service_inv.h>
#include <commutation_server.h>
#include <refclk.h>
#include <velocity_ctrl_client.h>
#include <velocity_ctrl_server.h>
#include <xscope_wrapper.h>
#include <profile.h>
#include <drive_modes.h>
#include <statemachine.h>
#include <profile_control.h>
#include <qei_client.h>
#include <internal_config.h>
//Configure your motor parameters in config/bldc_motor_config.h
#include <bldc_motor_config.h>
#include <rotary_sensor.h>

//#define ENABLE_xscope

on tile[IFM_TILE]: sensor_spi_interface pRotarySensor =
{
        {
            XS1_CLKBLK_3,
            XS1_CLKBLK_4,
            XS1_PORT_1I, //D0,    //mosi
            XS1_PORT_1J, //D1,    //sclk
            XS1_PORT_1F  //D2     //miso
        },

        XS1_PORT_1E //D3         //slave select
};

#define AMS_INIT_SETTINGS1  1    // Factory Setting 1
                                // NOISESET 0
                                // DIR      1   (CCW)
                                // UVW_ABI  0
                                // DAECDIS  0
                                // ABIBIN   0
                                // Dataselect 0
                                // PWMon    0

#define AMS_INIT_SETTINGS2  4    //UVWPP     001 (5)
                                //HYS       0
                                //ABIRES    0

#define ENABLE_XSCOPE

on stdcore[IFM_TILE]: clock clk_adc = XS1_CLKBLK_1;
on stdcore[IFM_TILE]: clock clk_pwm = XS1_CLKBLK_REF;

void xscope_initialise_1()
{
    xscope_register(2,
            XSCOPE_CONTINUOUS, "0 actual_velocity", XSCOPE_INT,	"n",
            XSCOPE_CONTINUOUS, "1 target_velocity", XSCOPE_INT, "n");
}

/* Test Profile Velocity function */
void profile_velocity_test(chanend c_velocity_ctrl)
{
	int target_velocity = 1000;	 		// rpm
	int acceleration 	= 1000;			// rpm/s
	int deceleration 	= 1000;			// rpm/s

#ifdef ENABLE_XSCOPE
	xscope_initialise_1();
#endif

	while(1){

	    set_profile_velocity( target_velocity, acceleration, deceleration, MAX_PROFILE_VELOCITY, c_velocity_ctrl);
	    delay_seconds(5);
        target_velocity *= -1;
	}
}

int main(void)
{
	// Motor control channels
	chan c_qei_p1, c_qei_p2;		                    // qei channels
	chan c_hall_p1, c_hall_p2;				            // hall channels
	chan c_commutation_p2;	                            // commutation channels
	chan c_pwm_ctrl, c_adctrig;							// pwm channels
	chan c_velocity_ctrl;								// velocity control channel
	chan c_watchdog; 									// watchdog channel

	par
	{

		/* Test Profile Velocity function */
		on tile[APP_TILE_1]:
		{
			profile_velocity_test(c_velocity_ctrl);			// test PVM on node

		}

		on tile[APP_TILE_1]:
		{

			/* Velocity Control Loop */
			{
				ctrl_par velocity_ctrl_params;
				filter_par sensor_filter_params;
				hall_par hall_params;
				qei_par qei_params;

				/* Initialize PID parameters for Velocity Control (defined in config/motor/bldc_motor_config.h) */
				init_velocity_control_param(velocity_ctrl_params);

				/* Initialize Sensor configuration parameters (defined in config/motor/bldc_motor_config.h) */
				init_hall_param(hall_params);
				init_qei_param(qei_params);

				/* Initialize sensor filter length */
				init_sensor_filter_param(sensor_filter_params);

				/* Control Loop */
				velocity_control(velocity_ctrl_params, sensor_filter_params, hall_params, \
					 qei_params, SENSOR_USED, c_hall_p2, c_qei_p2, c_velocity_ctrl, c_commutation_p2);
			}

		}

		/************************************************************
		 * IFM_CORE
		 ************************************************************/
		on tile[IFM_TILE]:
		{

		    initRotarySensor(pRotarySensor,  AMS_INIT_SETTINGS1,  AMS_INIT_SETTINGS2, 6250);

			par
			{

				/* PWM Loop */
			    {
#ifdef DC1K
			        // Turning off all MOSFETs for for initialization
                    disable_fets(p_ifm_motor_hi, p_ifm_motor_lo, 4);
#endif
				    do_pwm_inv_triggered(c_pwm_ctrl, c_adctrig, p_ifm_dummy_port, p_ifm_motor_hi, p_ifm_motor_lo, clk_pwm);
			    }

				/* Motor Commutation loop */
				{
					hall_par hall_params;
					qei_par qei_params;
					commutation_par commutation_params;
					int init_state;
					init_hall_param(hall_params);
					init_qei_param(qei_params);
					commutation_sinusoidal(c_hall_p1,  c_qei_p1, null, c_watchdog,
							null, c_commutation_p2, null, c_pwm_ctrl,
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
					p_ifm_encoder_hall_select_ext_d4to5 <: SET_PORT1_AS_HALL_PORT2_AS_QEI;
#endif
					run_hall(c_hall_p1, c_hall_p2, null, null, null, null, p_ifm_hall, hall_params); // channel priority 1,2..5

				}

				/* QEI Server */
				{
					qei_par qei_params;
					init_qei_param(qei_params);

					run_qei(c_qei_p1, c_qei_p2, null, null, null, null, p_ifm_encoder, qei_params);  		 // channel priority 1,2..5

				}

			}
		}

	}

	return 0;
}
