.. _SOMANET_Cyclic_Positioning_Control_with_EtherCAT_Demo_Quickstart:

SOMANET Cyclic Positioning Control with EtherCAT Demo Quickstart Guide
======================================================================

This simple demonstration shows how to control your motor using SOMANET EtherCAT motor control kit from a Linux PC. Only Cyclic Synchronous Positioning control mode is included with a simple linear profile generator. The CSP control mode is designed to achieve a desired motion trajectory by using various motion profiles and closing the control loop over EtherCAT. The slave controller in its turn is taking the generated at a fixed time interval (1ms) target position setpoints as a controller input and will be following them. 

Hardware setup
++++++++++++++

A minimal requirement for this application to run is having the complete SOMANET stack assembled consisting of the SOMANET Core, SOMANET COM-EtherCAT, and SOMANET IFM Drive DC 100 modules. The stack should be powered via the SOMANET IFM board. An example of a stack consisting of the SOMANET COM-EtherCAT, Core, and IFM Drive DC 100 boards is shown below. In this case the IFM DC 100 board can be supplied with 12 - 24 V DC power source. For the motor supplied with the kit required power supply voltage should be 24 Volts. For the best experience please make sure that your stabilized DC power supply is capable of delivering more that 2 Amperes of power. Please mind that at high motor accelerations starting current may be as high as 10 times the nominal.     

.. figure:: images/ethercat_stack.jpg
   :width: 400px
   :align: center

   Hardware Setup for SOMANET Cyclic Positioning Control with EtherCAT Demo

To setup the system:

   #. If you don't have the stack assembled, assemble it as shown in the image above. Make sure to connect the IFM side of the SOMANET core module to the IFM DC 100 board and COM side to the Core Debug Adapter (see markings on the Core module)
   #. Connect the xTAG-2 Adapter to the Core Debug Adapter.
   #. Connect the xTAG-2 to host PC. 
   #. Connect the motor supplied with the kit as shown in the image bellow.
   #. Connect the IFM DC 100 board to a 24 V DC power supply
   #. Connect one side of the Ethernet cable to the node and plug the RS-45 connector to your PC.
   #. Switch on the power supply. If everything is connected properly, drained current should not exceed 100mA. 

.. figure:: images/stack_and_motor.jpg
   :width: 400px
   :align: center

   Connecting the motor and cables to your kit


Import and build the application
++++++++++++++++++++++++++++++++

   #. Open xTIMEcomposer Studio and check that it is operating in online mode. Open the edit perspective (Window->Open Perspective->XMOS Edit).
   #. Locate the ``'EtherCAT Motorcontrol CSP Demo'`` item in the xSOFTip pane on the bottom left of the window and drag it into the Project Explorer window in xTIMEcomposer. This will also cause the modules on which this application depends to be imported as well. 
   #. Click on the ``app_demo_master_cyclic_position`` item in the Project Explorer plane then click on the build icon (hammer) in xTIMEcomposer. Check the Console window to verify that the application has built successfully. Note that you require the EtherLab IgH EtherCAT driver to be installed on your system to build the application.

For help in using xTIMEcomposer, try the xTIMEcomposer tutorial, which you can find by selecting Help->Tutorials from the xTIMEcomposer menu.

Note that the Developer Column in xTIMEcomposer Studio on the right hand side of your screen provides information on the xSOFTip components you are using. Select the ``sw_sncn_motorcontrol_ethercat_kit`` component in the Project Explorer, and you will see its description together with API documentation. Having done this, click the `back` icon until you return to this quickstart guide within the Developer Column.


Run the application
+++++++++++++++++++

When the application has been compiled, the next step is to run it on the Linux PC. Before doing that, make sure that the SOMANET EtherCAT stack is running a proper motor control software for the EtherCAT slave side, i.e. ``app_demo_slave_ethercat_motorcontrol``.  

   #. Make sure that the stack is recognized. For this you have to have the EtherCAT driver up and running. To start the driver on a Linux machine execute in a terminal the following command: ::

       sudo /etc/init.d/ethercat start

   #. To verify that the node is present in the system, type: ::

       ethercat slave 

   The output should indicate a presence of the SOMANET node and preoperational state if the slave side software is running: ::

       0  0:0  PREOP  +  SNCN SOMANET COM ECAT

   #. Navigate with the terminal to your compiled application binary on the hard disk. Then execute the application with super user rights: ::

       sudo ./demo-master-cyclic-position 

   #. The application will deploy the motor-specific configuration parameters over the EtherCAT and the rotor of the motor should make one rotation and hold the last position. In the terminal window you should be able to see the motor's feedback as current position, velocity, and torque ::

       actual position 12728 actual velocity 43 actual_torque 8.172973

   #. The debug console window in xTIMEcomposer will not display any message because the demo application is written to work with an EtherCAT master application and feedback is therefore provided via EtherCAT communication.


Next steps
++++++++++

As a next step you can run another EtherCAT Master Motorcontrol Demo. Two more control modes are offered: Cyclic Synchronous Velocity (``app_demo_master_cyclic_velocity``) and Cyclic Synchronous Torque (``app_demo_master_cyclic_torque``).

Examine the code
................

   #. In xTIMEcomposer navigate to the ``src`` directory under app_demo_master_cyclic_position and double click on the ``main.c`` file within it. The file will open in the central editor window.

   #. Find and examine the main function. At the beginning you'll find variables declarations that will be used to define your desired motion profile and provide you feedback from the motor. The ``slave_number`` variable is used when the nodes are operating in a multi-node setup.

   #. Before starting the main control routine you are required to initialise a set of parameters and to follow a motor starting state machine as defined in the CiA 402 directive (see the image bellow).

.. figure:: images/Ethercat_operating_state_machine.jpg
   :width: 400px
   :align: center

   Motorcontrol state machine

   #. ``init_master`` is taking care of the EtherCAT communication initialization. In case of the multi-node system the EtherCAT nodes can be configured from the ``ethercat_setup.h`` in the ``src`` directory. The default configuration allows you to get started with a single node setup without making any changes.

   #. ``initialize_torque`` is required to have a torque feedback, even if you are not using the torque control.

   #. The ``init_nodes`` routine will take care of loading your motor configuration(s) into the slaves via EtherCAT. All slave nodes are running the same software and can be configured for using different motors from the master side. The motor configurations are included in the ``motor_config`` folder, and the config files there have ``_N`` extensions to differentiate between various motors. When you specify a CONFIG_NUMBER in the ``SOMANET_C22_CTRLPROTO_SLAVE_HANDLES_ENTRY`` (defined in the ``ethercat_setup.h``), all corresponding configurations are being loaded to all the nodes. For the single-node setup only ``bldc_motor_config_1.h`` is used.

   #. ``set_operation_mode`` defines the control mode to be used. In this example we are using the Cyclic Synchronous Positioning mode (CSP).

   #. ``enable_operation`` is a part of the state machine control sequence as described above.

   #. ``initialize_position_profile_limits`` initializes position control limits as defined in your ``bldc_motor_config_1.h`` file.

   #. The motion control routine should be executed in a loop. In the example we perform one complete rotor rotation with a linear motion profile. The ``pdo_handle_ecat`` is a handler that takes care of a real-time information update over EtherCAT.  

   #. To start moving to a desired position we first need to identify where we are. Call the ``get_position_actual_ticks`` method to get the actual position. Note, that positioning control uses Hall or Encoder sensor ticks to close the loop. The Hall sensors provide a discrete feedback and the values between are interpolated based on the velocity. One magnetic rotation results in 4096 Hall sensor ticks. To perform one complete rotation the magnetic rotation setpoint value should be multiplied by the number of magnetic poles. In the kit the motor has 3 pole pairs, therefore the desired setpoint would be 4096 X 3 = 12288 that is added to the actual position to calculate the ``target_position`` for one complete rotation.

   #. After we defined how far we should move, we need to calculate number of steps required for the profile generator to complete the motion. For this the ``init_position_profile_params`` method is used that takes as an input the target position, actual position, desired profile velocity, and accelerations and decelerations to reach that velocity.

   #. The steps are then provided in a cyclic way to the motion profile generator (``generate_profile_position``) that calculates the immediate position setpoint (``position_ramp``) that is used as input for the positioning controller on the slave side (is sent over EtherCAT by the ``set_position_ticks`` function call). 

   #. To get the position, velocity and torque feedback from the controller the ``get_position_actual_ticks``, ``get_velocity_actual_rpm``, and ``get_torque_actual_mNm`` functions are used respectively.


   #. As an example for the state machine the methods as ``quick_stop_position``, ``renable_ctrl_quick_stop``, ``set_operation_mode``, ``enable_operation``, and ``shutdown_operation`` are included in the software but are not used. Please refer to the state machine diagram to include them properly when developing a custom application.


