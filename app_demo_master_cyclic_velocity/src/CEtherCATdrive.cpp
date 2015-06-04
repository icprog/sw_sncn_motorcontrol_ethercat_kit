/*
 * CC_EtherCAT_drivedrive.cpp
 *
 *  Created on: Jun 4, 2015
 *      Author: slava
 */

#include "CEtherCATdrive.h"




C_EtherCAT_drive::C_EtherCAT_drive() {
    // TODO Auto-generated constructor stub
}

C_EtherCAT_drive::~C_EtherCAT_drive() {
    // TODO Auto-generated destructor stub
}

void C_EtherCAT_drive::init_master_(int slave_ID, unsigned int total_no_of_slaves, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles){

    /* Initialize C_EtherCAT_drive Master */
    init_master(master_setup, slv_handles, total_no_of_slaves);

    /* Initialize all connected nodes with Mandatory Motor Configurations (specified in config/motor/)*/
    init_nodes(master_setup, slv_handles, total_no_of_slaves);

}

int C_EtherCAT_drive::set_operation_mode_(int operation_mode, int slave_ID, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, int total_no_of_slaves){
    return set_operation_mode(operation_mode, slave_ID, master_setup, slv_handles, total_no_of_slaves);
}

int C_EtherCAT_drive::enable_operation_(int slave_ID, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, int total_no_of_slaves){
    return enable_operation(slave_ID, master_setup, slv_handles, total_no_of_slaves);
}

int C_EtherCAT_drive::shutdown_operation_(int operation_mode, int slave_ID, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, int total_no_of_slaves){
    return shutdown_operation(operation_mode, slave_ID, master_setup, slv_handles, total_no_of_slaves);
}

int C_EtherCAT_drive::calculate_profile_steps_(int target_velocity, int actual_velocity, int acceleration, int deceleration, int slave_ID, ctrlproto_slv_handle *slv_handles){
    return init_velocity_profile_params(target_velocity, actual_velocity, acceleration, deceleration, slave_ID, slv_handles);
}

void C_EtherCAT_drive::update_PDOs(master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, unsigned int total_no_of_slaves){

    pdo_handle_ecat(master_setup, slv_handles, total_no_of_slaves);
}

int C_EtherCAT_drive::calculate_next_setpoint(int step, int slave_ID, ctrlproto_slv_handle *slv_handles){
    return generate_profile_velocity(step, slave_ID, slv_handles);
}

void C_EtherCAT_drive::set_velocity(int target_velocity, int slave_ID, ctrlproto_slv_handle *slv_handles){
    set_velocity_rpm(target_velocity, slave_ID, slv_handles);
}

int C_EtherCAT_drive::get_velocity(int slave_ID, ctrlproto_slv_handle *slv_handles){
    return get_velocity_actual_rpm(slave_ID, slv_handles);
}

int C_EtherCAT_drive::get_position(int slave_ID, ctrlproto_slv_handle *slv_handles){
    return get_position_actual_ticks(slave_ID, slv_handles);
}

int C_EtherCAT_drive::get_torque(int slave_ID, ctrlproto_slv_handle *slv_handles){
    return get_torque_actual_mNm(slave_ID, slv_handles);
}

int C_EtherCAT_drive::quick_stop_velocity_(int slave_ID, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, int total_no_of_slaves){
    return quick_stop_velocity(slave_ID, master_setup, slv_handles, total_no_of_slaves);
}

int C_EtherCAT_drive::renable_ctrl_quick_stop_(int operation_mode, int slave_ID, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, int total_no_of_slaves){
    return renable_ctrl_quick_stop(operation_mode, slave_ID, master_setup, slv_handles, total_no_of_slaves);
}

int C_EtherCAT_drive::quick_stop_node(int operation_mode, int slave_ID, master_setup_variables_t *master_setup, ctrlproto_slv_handle *slv_handles, int total_no_of_slaves){

    //ToDo implement control mode cases

    /* Quick stop velocity mode (for emergency) */
    quick_stop_velocity(slave_ID, master_setup, slv_handles, total_no_of_slaves);

    /* Regain control of node to continue after quick stop */
    renable_ctrl_quick_stop(operation_mode, slave_ID, master_setup, slv_handles, total_no_of_slaves);

    /* Initialize the node specified with ECAT_SLAVE_0 with CSV configurations */
    set_operation_mode(operation_mode, slave_ID, master_setup, slv_handles, total_no_of_slaves);

    /* Enable operation of node in XXX mode */
    return enable_operation(slave_ID, master_setup, slv_handles, total_no_of_slaves);
}

