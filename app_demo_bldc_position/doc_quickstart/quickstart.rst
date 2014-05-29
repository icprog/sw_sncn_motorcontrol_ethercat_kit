.. _SOMANET_IFM_Drive_Positioning_Control_Demo_Quickstart:

SOMANET Slave Firmware Update Demo Quickstart Guide
===================================================

sw_sncn_motorcontrol_ethercat_kit : Quick Start Guide
-----------------------------------------------------

This demonstrative application illustrates usage of ``module_ctrl_loops`` to do position control of a brushless dc motor. Position loop can be closed with a positioning feedback either from a HALL sensor/ QEI Sensor or any other positioning sensor.

Hardware Setup
++++++++++++++

On how to setup the hardware please refer to the `XMOS Motor & Motion Control Kit Hardware Guide <http://www.python.org/>`_ To run this application no SOMANET COM-EtherCAT module is required but it won't affect the application performance if you have the EtherCAT module already installed. The IFM DC 100 board can be supplied with 12 - 24 V DC power source. Please connect the motor prior to powering up the stack. 
   
To connect the motor:



