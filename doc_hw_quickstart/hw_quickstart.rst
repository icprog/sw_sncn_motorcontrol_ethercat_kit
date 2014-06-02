.. _XMOS_Motor_Motion_Control_Kit_User_Guide:

The XMOS MOTOR & MOTION Control Kit Hardware
========================================================

The XMOS Motor & Motion Control Kit contains a combination of hardware and software modules developed by Synapticon using SOMANET technology and XMOS multicore microcontrollers, with a smaple BLDC motor. The kit can be assembled in different configurations depending on the type of motor and communications interface you require. Currently there are two hardware configurations:

   * Drive DC 100 with EtherCAT comms and BLDC motor with Hall sensor encoding
   * Drive DC 100 with EtherCAT comms and BLDC motor with Quadrature encoder and Hall sensor encoding

For information on assembling the SOMANET hardware modules, see the Hardware Quick Start Guide.

For information on connecting the motor, see the Connecting Motor Guide.

The software modules available for the kit include:

   * Motor commutation with position, velocity & torque control loops
   * Support for Hall and Quadrature encoder sensors
   * EtherCAT communication[1]
   * CiA402 drive profile

[1] To evaluate the EtherCAT module an EtherCAT master is required which is currently only available free of charge on Linux platforms. Windows developers must purchase a TwinCAT license from Beckhoff Automation GmbH, or a similar solution. 

For information on running the sotware modules, see the Software Quick Start Guide.

.. _XMOS_Motor_Motion_Control_Kit_User_Guide_Hardware:

Hardware Quick Start Guide
----------------------------

This guide shows how to assemble the Motor & Motion Control Kit modules in the following configurations:

   * Drive DC 100 with COM EtherCAT module and xTAG debug adapter
   * Drive DC 100 with COM EtherCAT module and no debug adapter
   * Drive DC 100 without COM EtherCAT module

.. _XMOS_Motor_Motion_Control_Kit_User_Guide_Hardware_EtherCAT:

Setting up with COM-EtherCAT module
+++++++++++++++++++++++++++++++++++

This setup is required if you want to develop an EtherCAT based slave application for SOMANET devices. Before you start, make sure that you have all hardware components and assembly parts at hand. You will require the following items:

   #. One SOMANET IFM Drive DC 100 board.
   #. One SOMANET Core C22 module.
   #. One Core to xTAG-2 Debug Adapter.
   #. One XMOS xTAG-2 Adapter
   #. Two 25mm M2 screws
   #. Four 4mm metal spacers
   #. Two 3.5mm plastic spacers
   #. Two M2 nuts.

The image below illustrates the required set for this hardware setup.

.. figure:: images/HW_set_complete.jpg
   :width: 300px
   :align: center

   Required components and assembly parts

It is recommended to start assembly from the IFM Drive DC 100 board. Place the long metal screws in two holes as shown in image below. Use 4 mm metal spaces between the IFM and the Core modules.

.. figure:: images/assembly_p1.jpg
   :width: 300px
   :align: center

   Step 1

Then proceed with the SOMANET Core module. Hold the screws with your fingers to prevent them from falling out during the assembly procedure. Make sure that you connect the IFM side connector of the Core module to the IFM Drive DC 100 board. The IFM connector of the Core module has ``IF`` marking next to it. As a result you should see the ``COM`` connector being on top. For mounting the Core to xTAG-2 Adapter in the next step please use the white plastic 3.5 mm spacers. 

.. figure:: images/assembly_p2.jpg
   :width: 300px
   :align: center

   Step 2

When mounting the Core to xTAG-2 Adapter, hold the IFM connector side with your thumb to prevent from unplugging. Also be careful with the golden pins of the Core to xTAG-2 Adapter not to bend them or to break. The golden pins should have a reliable contact at the end with the corresponding pads of the SOMANET core C22 module.

.. figure:: images/assembly_p3.jpg
   :width: 300px
   :align: center

   Step 3

Use two metal 4 mm spacers for mounting the COM-EtherCAT board on top of the Core to xTAG-2 Adapter. 

.. figure:: images/assembly_p9.jpg
   :width: 300px
   :align: center

   Step 4

When mounting the COM-EtherCAT board, press the IFM connector side of the Core module with your thumb to prevent from unplugging. Press together the modules on the COM connector side and make sure that the all modules are properly plugged. Then fix the screws with the nuts.

.. figure:: images/assembly_p4.jpg
   :width: 300px
   :align: center

   Step 5

All you need now to start programming is to connect the XMOS xTAG-2 adapter as shown in image below. Note that the USB cable and a power supply are not included into the kit. 


.. figure:: images/assembly_p7.jpg
   :width: 300px
   :align: center

   Step 6

.. _XMOS_Motor_Motion_Control_Kit_User_Guide_Hardware_EtherCAT_No_xTAG:

Setting up with COM-EtherCAT module without Core to xTAG-2 Adapter
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

This setup is useful if software debugging or xSCOPE functionality is not required and you have previously flashed the SOMANET node with a firmware that includes update functionality via EtherCAT. 

You will require the following items:

   #. One SOMANET IFM Drive DC 100 board.
   #. One SOMANET Core C22 module.
   #. One XMOS XTAG2 Adapter
   #. Two 25mm M2 screws
   #. Four 4mm metal spacers
   #. Two M2 nuts.

The image below illustrates the required set for this hardware setup.

.. figure:: images/HW_set_complete_w_o_debug.jpg
   :width: 300px
   :align: center

   Required components and assembly parts

To assemble the stack for using with SOMANET COM-EtherCAT without the Core to XTAG2 Adapter please perform step 1 as described in **Setting up with COM-EtherCAT module** section. Then proceed with the SOMANET Core module. Please hold the screws with your fingers to prevent them from falling out during the assembly procedure. Make sure that you connected the IFM side connector of the Core module to the IFM Drive DC 100 board. The IFM connector of the Core module has ``IF`` marking next to it. As a result you should see the ``COM`` connector being on top. For mounting the COM-EtherCAT module in the next step please use the metal 4 mm spacers. 

.. figure:: images/assembly_p5.jpg
   :width: 300px
   :align: center

   Step 2

When mounting the COM-EtherCAT board, hold the IFM connector side of the Core module with your thumb to prevent from unplugging. Press together the modules on the COM connector side and make sure that the all modules are properly plugged. Then fix the screws with the nuts.

.. figure:: images/assembly_p6.jpg
   :width: 300px
   :align: center

   Step 3

All you need now to start programming the SOMANET stack is to connect the Ethernet cable from the kit to one of the ports of the COM-EtherCAT module and to an Ethernet port of your PC. Note that the power supply is not included into the kit. 


.. _XMOS_Motor_Motion_Control_Kit_User_Guide_Hardware_No_EtherCAT:

Setting up without COM-EtherCAT module
++++++++++++++++++++++++++++++++++++++

This is a minimal hardware setup to start working with the motor control software. Before you start, make sure that you have all hardware components and assembly parts at hand. You will require the following items:

   #. One SOMANET IFM Drive DC 100 board.
   #. One SOMANET Core C22 module.
   #. One Core to xTAG-2 Debug Adapter.
   #. One XMOS xTAG-2 Adapter
   #. Two 25mm M2 screws
   #. Two 4mm metal spacers
   #. Two 3.5mm plastic spacers
   #. Two M2 nuts.

The image below illustrates the required set for this hardware setup.

.. figure:: images/HW_set_minimal.jpg
   :width: 300px
   :align: center

   Required components and assembly parts

It is recommended to start assembly from the IFM Drive DC 100 board. Please place the long metal screws in two holes as shown in image below. Use 4 mm metal spaces between the IFM and the Core modules.

.. figure:: images/assembly_p1.jpg
   :width: 300px
   :align: center

   Step 1

Then proceed with the SOMANET Core module. Please hold the screws with your fingers to prevent them from falling out during the assembly procedure. Make sure that you connected the IFM side connector of the Core module to the IFM Drive DC 100 board. The IFM connector of the Core module has ``IF`` marking next to it. As a result you should see the ``COM`` connector being on top. For mounting the Core to xTAG-2 Adapter in the next step please use the white plastic 3.5 mm spacers. 

.. figure:: images/assembly_p2.jpg
   :width: 300px
   :align: center

   Step 2

When mounting the Core to xTAG-2 Adapter please fixate the IFM connector side with your thumb to prevent from unplugging. Also be careful with the golden pins of the Core to xTAG-2 Adapter not to bend them or to break. The golden pins should have a reliable contact at the end with the corresponding pads of the SOMANET core C22 module. When successfully mounted, please fixate the stack with the metal nuts. The stack is ready to use.

.. figure:: images/assembly_p3.jpg
   :width: 300px
   :align: center

   Step 3

All you need now to start programming is to connect the XMOS xTAG-2 adapter as shown in image below. Note that the USB cable and a power supply are not included into the kit. 


.. figure:: images/assembly_p8.jpg
   :width: 300px
   :align: center

   Step 4

.. _XMOS_Motor_Motion_Control_Kit_User_Guide_Motor:

Connecting the Motor Guide
---------------------------

The Motor & Motion Control Kit includes an sample motor that you can use the software modules to run.



.. _XMOS_Motor_Motion_Control_Kit_User_Guide_Software:

Software Quick Start Guide
------------------------------

The Motor & Motion Control Kit includes a set of software modules that run on the hardware, including:

   * Hall Sensor
   * Motor commutation
   * AN Other
   
We recommend that you run the Hall Sensor application first to test that the motor runs correctly.

The software is delivered as individual components within the xTIMEcomposer Studio development tools, which are available free of charge from the XMOS website: http://www.xmos.com/xtimecomposer

Installing xTIMEcomposer
+++++++++++++++++++++++++++

Details about downloading and installing the tools

Importing and running the Hall Sensor application
++++++++++++++++++++++++++++++++++++++++++++++++++

Drag in from xSOFTip Browser.

Copy XN file to project.

Build and run on hardware.

Importing another application
+++++++++++++++++++++++++++++++

Import a motor commutation app.

Build and run.

.. _XMOS_Motor_Motion_Control_Kit_User_Guide_Next:

Next steps
------------------------------


Configuring your own motor
+++++++++++++++++++++++++++

Details about how to configure your own motor.






