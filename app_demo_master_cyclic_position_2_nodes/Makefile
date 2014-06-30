UNAME := $(shell uname)
SOURCE_DIRS = src
SOURCES += src/main.c 
FLAGS = -static
FLAGS += -std=c99

MASTER_INSTALLDIR := /opt/etherlab

LIB_DIRS += -L $(MASTER_INSTALLDIR)/lib
LIBS += -lsncn_ctrlproto -lsncn_motor_drive
LIBS += -lethercat -lm

INCLUDE_DIRS += -I $(MASTER_INSTALLDIR)/include
INCLUDE_DIRS += -I ../motor_config

# EXTERNAL INCLUDES #
INCLUDE_DIRS += -I ../inc_ctrlproto-common
INCLUDE_DIRS += -I ../module_profile/include
INCLUDE_DIRS += -I ../module_motorcontrol_common/include
INCLUDE_DIRS += -I ../module_hall/include
INCLUDE_DIRS += -I ../module_qei/include
INCLUDE_DIRS += -I ../lib_linux_ctrlproto/include
INCLUDE_DIRS += -I ../lib_linux_motor_drive/include
LIB_DIRS += -L ../lib_linux_ctrlproto/lib
LIB_DIRS += -L ../lib_linux_motor_drive/lib

# DEVELOPERS ONLY #
ifeq ($(MAKECMDGOALS),develop)
CTRL_PROTO_PATH := ../../sc_sncn_ctrlproto
MOTORCONTROL_PATH := ../../sc_sncn_motorcontrol
INCLUDE_DIRS += -I $(CTRL_PROTO_PATH)/inc_ctrlproto-common
INCLUDE_DIRS += -I $(CTRL_PROTO_PATH)/lib_linux_ctrlproto/include
INCLUDE_DIRS += -I $(CTRL_PROTO_PATH)/lib_linux_motor_drive/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_profile/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_motorcontrol_common/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_hall/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_qei/include
LIB_DIRS += -L ../../sc_sncn_ctrlproto/lib_linux_ctrlproto/lib
LIB_DIRS += -L ../../sc_sncn_ctrlproto/lib_linux_motor_drive/lib

endif
# END DEVELOPERS ONLY #
# END EXTERNAL INCLUDES #

all develop:
		@echo Bulding dependency: lib_linux_ctrlproto...
		@cd $(CTRL_PROTO_PATH)/lib_linux_ctrlproto; make develop
		@echo 
		@echo Bulding dependency: lib_linux_motor_drive...
		@cd $(CTRL_PROTO_PATH)/lib_linux_motor_drive; make develop
		@echo 
		@echo Bulding application...
ifneq "$(UNAME)" "Linux"
		@echo
		@echo "------------------------------------------------------------------------------------------------------"
		@echo "||Your System isn't Linux, so the Master Application for the Linux Ethercat Master can not be built.||"
		@echo "------------------------------------------------------------------------------------------------------"
		@echo
else
		@if test -d $(MASTER_INSTALLDIR); \
		then \
		mkdir bin -p;\
		gcc $(SOURCES) $(LIBS) $(INCLUDE_DIRS) $(LIB_DIRS) $(FLAGS) -o bin/$(notdir $(shell pwd)); \
		echo ;\
		echo "Run you application by typing: sudo bin/$(notdir $(shell pwd))" ;\
		echo ;\
		else echo \
		"\n\
		-----------------------------------------------------------------------------------------------------------------\n\
		||Ethercat Master is not installed at given directory if you installed it elsewhere change location in Makefile||\n\
		-----------------------------------------------------------------------------------------------------------------\n";\
		fi
endif

clean:
	rm -f bin/$(notdir $(shell pwd))
