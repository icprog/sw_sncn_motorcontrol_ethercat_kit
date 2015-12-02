/* INCLUDE BOARD SUPPORT FILES FROM module_board-support */
#include <COM_ECAT-rev-a.inc>
#include <CORE_C22-rev-a.inc>
#include <IFM_DC100-rev-b.inc>

/**
 * @file test_ethercat-mode.xc
 * @brief Test illustrates usage of Motor Control with Ethercat
 * @author Synapticon GmbH (www.synapticon.com)
 */

#include <print.h>

#include <qei_service.h>
#include <hall_service.h>
#include <pwm_service.h>
#include <commutation_service.h>
#include <gpio_server.h>
#include <adc_server_ad7949.h>

#include <velocity_ctrl_service.h>
#include <position_ctrl_service.h>
#include <torque_ctrl_service.h>

#include <ecat_motor_drive.h>

#include <ethercat.h>
#include <flash_somanet.h>
#include <comm.h>


 //Configure your default motor parameters in config/bldc_motor_config.h
#include <bldc_motor_config.h>
#include <qei_config.h>
#include <hall_config.h>
#include <commutation_config.h>
#include <control_config.h>



//on tile[IFM_TILE]: clock clk_adc = XS1_CLKBLK_1;
//on tile[IFM_TILE]: clock clk_pwm = XS1_CLKBLK_REF;
//on tile[IFM_TILE]: clock clk_biss = XS1_CLKBLK_2 ;

ethercat_interface_t ethercat_interface = SOMANET_COM_ETHERCAT_PORTS;

port p_ifm_ext_d[4] = { GPIO_D0, GPIO_D1, GPIO_D2, GPIO_D3 };


PwmPorts pwm_ports = PWM_PORTS;
WatchdogPorts wd_ports = WATCHDOG_PORTS;
ADCPorts adc_ports = ADC_PORTS;
FetDriverPorts fet_driver_ports = FET_DRIVER_PORTS;
HallPorts hall_ports = HALL_PORTS;
QEIPorts encoder_ports = ENCODER_PORTS;

int main(void)
{
    /* Motor control channels */
    chan c_adctrig, c_pwm_ctrl;

    interface WatchdogInterface i_watchdog;
    interface CommutationInterface i_commutation[5];
    interface ADCInterface i_adc;
    interface HallInterface i_hall[5];
    interface QEIInterface i_qei[5];

    chan c_gpio_p1, c_gpio_p2;  // gpio digital channels

    interface TorqueControlInterface i_torque_control;
    interface PositionControlInterface i_position_control;
    interface VelocityControlInterface i_velocity_control;

    /* EtherCat Communication channels */
    chan coe_in;          // CAN from module_ethercat to consumer
    chan coe_out;         // CAN from consumer to module_ethercat
    chan eoe_in;          // Ethernet from module_ethercat to consumer
    chan eoe_out;         // Ethernet from consumer to module_ethercat
    chan eoe_sig;
    chan foe_in;              // File from module_ethercat to consumer
    chan foe_out;             // File from consumer to module_ethercat
    chan pdo_in;
    chan pdo_out;
    chan c_nodes[1], c_flash_data; // Firmware channels


    par
    {
        /************************************************************
         *                          COM_TILE
         ************************************************************/

        /* Ethercat Communication Handler Loop */
        on tile[COM_TILE] :
        {
            ecat_init(ethercat_interface);
            ecat_handler(coe_out, coe_in, eoe_out, eoe_in, eoe_sig, foe_out,
                         foe_in, pdo_out, pdo_in, ethercat_interface);
        }

        /* Firmware Update Loop over Ethercat */
        on tile[COM_TILE] :
        {
            firmware_update_loop(p_spi_flash, foe_out, foe_in, c_flash_data, c_nodes, null);
        }

        /* Ethercat Motor Drive Loop */
        on tile[1] :
        {
            ecat_motor_drive(pdo_out, pdo_in, coe_out, i_commutation[3], i_hall[4], i_qei[4],
                    i_torque_control, i_velocity_control, i_position_control, c_gpio_p1);
        }

        on tile[2]:
        {
            par
            {
                /* Position Control Loop */
                {
                     ControlConfig position_ctrl_params;
                     init_position_control_config(position_ctrl_params); // Initialize PID parameters for Position Control

                     /* Control Loop */
                     position_control_service(position_ctrl_params, i_hall[1], i_qei[1], i_position_control, i_commutation[0]);
                }

                /* Velocity Control Loop */
                {
                    ControlConfig velocity_ctrl_params;
                    /* Initialize PID parameters for Velocity Control (defined in config/motor/bldc_motor_config.h) */
                    init_velocity_control_config(velocity_ctrl_params);

                    /* Control Loop */
                    velocity_control_service(velocity_ctrl_params, i_hall[2], i_qei[2], i_velocity_control, i_commutation[1]);
                }

                /* Torque Control Loop */
                {
                    /* Torque Control Loop */
                    ControlConfig torque_ctrl_params;
                    init_torque_control_config(torque_ctrl_params);  // Initialize PID parameters for Torque Control

                    /* Control Loop */
                    torque_control_service( torque_ctrl_params, i_adc, i_commutation[2], i_hall[3], i_qei[3], i_torque_control);
                }

            }
        }

        /************************************************************
         *                          IFM_TILE
         ************************************************************/
        on tile[IFM_TILE]:
        {
            par
            {
                /* ADC Loop */
                adc_service(i_adc, adc_ports, c_adctrig);

                /* PWM Loop */
                pwm_triggered_service(c_pwm_ctrl, c_adctrig, pwm_ports);

                /* Watchdog Server */
                 watchdog_service(i_watchdog, wd_ports);

                /* Hall Server */
                {
                    HallConfig hall_config;
                    init_hall_config(hall_config);

                    hall_service(i_hall, hall_ports, hall_config);
                }

                /* QEI Server */
                {
                    QEIConfig qei_config;
                    init_qei_config(qei_config);

                    qei_service(i_qei, encoder_ports, qei_config);
                }

                /* Motor Commutation loop */
                 {
                     CommutationConfig commutation_config;
                     init_commutation_config(commutation_config);

                     commutation_service(i_hall[0], i_qei[0], null, i_watchdog, i_commutation,
                             c_pwm_ctrl, fet_driver_ports, commutation_config);
                 }

                /* GPIO Digital Server */
                gpio_digital_server(p_ifm_ext_d, c_gpio_p1, c_gpio_p2);

            }
        }

    }
    return 0;
}
