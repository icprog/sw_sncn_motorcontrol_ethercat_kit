/*
 * CEtherCATdrive.h
 *
 *  Created on: Jun 4, 2015
 *      Author: slava
 */

#ifndef CETHERCATDRIVE_H_
#define CETHERCATDRIVE_H_

extern "C" {
    #include <ctrlproto_m.h>
    #include <ecrt.h>
    #include <profile.h>
    #include <drive_function.h>
    #include <motor_define.h>

}

class C_EtherCAT_drive {
    public:
        C_EtherCAT_drive();
        virtual ~C_EtherCAT_drive();
        /* List your slave devices */
        enum {ECAT_SLAVE_0};
        /* Initialize Ethercat Master */
        void init_master_(int slave_ID, unsigned int total_no_of_slaves, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles);
        /* Initialize the node specified with a slave_ID with a specific cyclic control mode*/
        int set_operation_mode_(int operation_mode, int slave_ID, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, int total_no_of_slaves);
        int enable_operation_(int slave_ID, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, int total_no_of_slaves);
        int shutdown_operation_(int operation_mode, int slave_ID, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, int total_no_of_slaves);
        /* Initialize velocity profile parameters */
        int calculate_profile_steps_(int target_velocity, int actial_velocity, int acceleration, int deceleration, int slave_ID, ctrlproto_slv_handle *slv_handles);
        /* Update the process data (EtherCat packets) sent/received from the node */
        void update_PDOs(master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, unsigned int total_no_of_slaves);
        int calculate_next_setpoint(int step, int slave_number, ctrlproto_slv_handle *slv_handles);
        void set_velocity(int target_velocity, int slave_ID, ctrlproto_slv_handle *slv_handles);
        int get_velocity(int slave_ID, ctrlproto_slv_handle *slv_handles);
        int get_position(int slave_ID, ctrlproto_slv_handle *slv_handles);
        int get_torque(int slave_ID, ctrlproto_slv_handle *slv_handles);
        int quick_stop_velocity_(int slave_ID, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, int total_no_of_slaves);
        int renable_ctrl_quick_stop_(int operation_mode, int slave_ID, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, int total_no_of_slaves);
        int quick_stop_node(int control_mode, int slave_ID, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, int total_no_of_slaves);
};

#endif /* CETHERCATDRIVE_H_ */
