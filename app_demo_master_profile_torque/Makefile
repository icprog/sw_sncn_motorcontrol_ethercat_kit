UNAME := $(shell uname)
SOURCE_DIRS = src

MASTER_INSTALLDIR := /opt/etherlab

SOURCES += src/main.c 
INCLUDE_DIRS += -I ../../sc_sncn_ctrlproto/inc_ctrlproto-common   
INCLUDE_DIRS += -I config/motor
INCLUDE_DIRS += -I ../../sc_sncn_motorcontrol/module_profile/include
INCLUDE_DIRS += -I ../../sc_sncn_ctrlproto/lib_linux_motor_drive/include
INCLUDE_DIRS += -I $(MASTER_INSTALLDIR)/include
INCLUDE_DIRS += -I ../../sc_sncn_ctrlproto/lib_linux_ctrlproto/include
INCLUDE_DIRS += -I ../../sc_sncn_motorcontrol/module_common/include
LIB_DIRS += -L $(MASTER_INSTALLDIR)/lib
LIB_DIRS += -L ../../sc_sncn_ctrlproto/lib_linux_ctrlproto/lib
LIB_DIRS += -L ../../sc_sncn_ctrlproto/lib_linux_motor_drive/lib

LIBS += -lsncn_ctrlproto -lsncn_motor_drive
LIBS += -lethercat -lm

FLAGS = -static
FLAGS += -std=c99

all:
		@cd ../../sc_sncn_ctrlproto/lib_linux_ctrlproto; make
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
		gcc $(SOURCES) $(LIBS) $(INCLUDE_DIRS) $(LIB_DIRS) $(FLAGS) -o bin/demo-master-profile-torque; \
		else echo \
		"\n\
		-----------------------------------------------------------------------------------------------------------------\n\
		||Ethercat Master is not installed at given directory if you installed it elsewhere change location in Makefile||\n\
		-----------------------------------------------------------------------------------------------------------------\n";\
		fi
endif

##Add clean Master Application
clean: masterAppClean
masterAppClean:
	rm -f bin/demo-master-profile-torque