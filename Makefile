# This variable should contain a space separated list of all
# the directories containing buildable applications (usually
# prefixed with the app_ prefix)
#
# If the variable is set to "all" then all directories that start with app_
# are built.
BUILD_SUBDIRS = demo-master-cyclic-position demo-master-cyclic-torque demo-master-cyclic-velocity demo-master-homing demo-master-profile-position demo-master-profile-torque demo-master-profile-velocity demo-slave-bldc-homing demo-slave-bldc-position demo-slave-bldc-torque demo-slave-bldc-velocity demo-slave-brushed-dc-position demo-slave-brushed-dc-velocity demo-slave-ethercat-motorcontrol

XMOS_MAKE_PATH ?= ..
include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.toplevel
