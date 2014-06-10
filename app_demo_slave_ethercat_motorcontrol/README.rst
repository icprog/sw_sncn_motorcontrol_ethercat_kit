EtherCAT slave motorcontrol demo
================================

:scope: Example
:description: This example demonstrates how to implement a motorcontrol software for the slave side to be used for EtherCAT based control. The example includes all control modes that can be freely selected from the master side.
:keywords: COM-EtherCAT, Motorcontrol, EtherCAT slave
:boards: SOMANET EtherCAT motorcontrol kit

Description
-----------

This demonstrative application illustrates usage of Motor Control with Ethercat. It includes such control modes as Syclic Synchronous Positioning (CSP), Syclic Synchronous Velocity (CSV), and Syclic Synchronous Torque (CST) as well as Profile Positioning Mode (PPM), Profile Velocity Mode (PVM), and Profile Torque Mode (PTM). The cyclic modes require the motion control loop to be closed from the Master side, when the profile modes implements ramps localy on the slave side and do not provide real-time feedback till the control task is finished (the motion profile is executed).



