/*****************************************************************************
 *
 *  $Id: main.c,v 6a6dec6fc806 2012/09/19 17:46:58 fp $
 *
 *  Copyright (C) 2007-2009  Florian Pose, Ingenieurgemeinschaft IgH
 *
 *  This file is part of the IgH EtherCAT Master.
 *
 *  The IgH EtherCAT Master is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License version 2, as
 *  published by the Free Software Foundation.
 *
 *  The IgH EtherCAT Master is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with the IgH EtherCAT Master; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  ---
 *
 *  The license mentioned above concerns the source code only. Using the
 *  EtherCAT technology and brand is only permitted in compliance with the
 *  industrial property and similar rights of Beckhoff Automation GmbH.
 *
 ****************************************************************************/
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <time.h>
#include <stdbool.h>
#include <limits.h>
/****************************************************************************/

#include "ecrt.h"

/****************************************************************************/

// Application parameters
#define FREQUENCY 100
#define PRIORITY 1
#define NSEC_PER_SEC (1000000000L)

// Optional features
#define CONFIGURE_PDOS  1
#define SDO_ACCESS      0

/****************************************************************************/

// EtherCAT
static ec_master_t *master = NULL;
static ec_master_state_t master_state = {};

static ec_domain_t *domain1 = NULL;
static ec_domain_state_t domain1_state = {};

static ec_slave_config_t *sc_ana_in = NULL;
static ec_slave_config_state_t sc_ana_in_state = {};

// Timer
static unsigned int sig_alarms = 0;
static unsigned int user_alarms = 0;

/****************************************************************************/

// process data
static uint8_t *domain1_pd = NULL;

#define SomaSlavePos 0, 0
#define SomaEtherCAT  0x000022d2, 0x00000201
/* Send Data */
// 16 bit
#define HEX88CC 0x88cc
#define HEXCC88 0xcc88
// 8 bit
#define HEXF1 0xF1
#define HEX1F 0x1F
// 16 bit
#define HEX2244 0x2244
#define HEX4422 0x4422
// 32 bit
#define HEX4D4D5E5E 0x4d4d5e5e
#define HEX5E5E4D4D 0x5e5e4d4d
// 32 bit
#define HEX1D1D2E2E 0x1d1d2e2e
#define HEX2E2E1D1D 0x2e2e1d1d
#define DIFFMAX 20

static unsigned int off_in_pdo1 = 0; /* 16 bit */
static unsigned int off_in_pdo2 = 0; /* 8 bit */
static unsigned int off_in_pdo3 = 0; /* 32 bit */
static unsigned int off_in_pdo4 = 0; /* 32 bit */
static unsigned int off_in_pdo5 = 0; /* 32 bit */

static unsigned int off_out_pdo1 = 0; /* 16 bit */
static unsigned int off_out_pdo2 = 0; /* 8 bit */
static unsigned int off_out_pdo3 = 0; /* 32 bit */
static unsigned int off_out_pdo4 = 0; /* 32 bit */
static unsigned int off_out_pdo5 = 0; /* 32 bit */

const static ec_pdo_entry_reg_t domain1_regs[] = {
    /* Out */
    {SomaSlavePos,  SomaEtherCAT, 0x1600, 1, &off_out_pdo1},
	{SomaSlavePos,  SomaEtherCAT, 0x1600, 2, &off_out_pdo2},
	{SomaSlavePos,  SomaEtherCAT, 0x1600, 3, &off_out_pdo3},
	{SomaSlavePos,  SomaEtherCAT, 0x1600, 4, &off_out_pdo4},
	{SomaSlavePos,  SomaEtherCAT, 0x1600, 5, &off_out_pdo5},
	/* In */    
	{SomaSlavePos,  SomaEtherCAT, 0x1a00, 1, &off_in_pdo1},
	{SomaSlavePos,  SomaEtherCAT, 0x1a00, 2, &off_in_pdo2},
	{SomaSlavePos,  SomaEtherCAT, 0x1a00, 3, &off_in_pdo3},
	{SomaSlavePos,  SomaEtherCAT, 0x1a00, 4, &off_in_pdo4},
	{SomaSlavePos,  SomaEtherCAT, 0x1a00, 5, &off_in_pdo5},
    {}
};

//static unsigned int counter = 0;
//static unsigned int blink = 0;
/*****************************************************************************/

#if CONFIGURE_PDOS
ec_pdo_entry_info_t slave_0_pdo_entries[] = {
    {0x1600, 0x01, 16}, /* ECAT Out1 */
    {0x1600, 0x02, 8},  /* ECAT Out2 */
    {0x1600, 0x03, 16}, /* ECAT Out2 */
    {0x1600, 0x04, 32}, /* ECAT Out2 */
    {0x1600, 0x05, 32}, /* ECAT Out2 */
    {0x1a00, 0x01, 16}, /* ECAT In1 */
    {0x1a00, 0x02, 8},  /* ECAT In2 */
    {0x1a00, 0x03, 32}, /* ECAT In2 */
    {0x1a00, 0x04, 32}, /* ECAT In2 */
    {0x1a00, 0x05, 16}, /* ECAT In2 */
};

ec_pdo_info_t slave_0_pdos[] = {
    {0x1600, 5, slave_0_pdo_entries + 0}, /* Outputs */
    {0x1a00, 5, slave_0_pdo_entries + 5}, /* Inputs */
};

ec_sync_info_t slave_0_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_DISABLE},
    {3, EC_DIR_INPUT, 1, slave_0_pdos + 1, EC_WD_DISABLE},
    {0xff}
};
#endif

/*****************************************************************************/

#if SDO_ACCESS
static ec_sdo_request_t *sdo;
#endif

/*****************************************************************************/

void check_domain1_state(void)
{
    ec_domain_state_t ds;

    ecrt_domain_state(domain1, &ds);

    if (ds.working_counter != domain1_state.working_counter)
        printf("Domain1: WC %u.\n", ds.working_counter);
    if (ds.wc_state != domain1_state.wc_state)
        printf("Domain1: State %u.\n", ds.wc_state);

    domain1_state = ds;
}

/*****************************************************************************/

void check_master_state(void)
{
    ec_master_state_t ms;

    ecrt_master_state(master, &ms);

    if (ms.slaves_responding != master_state.slaves_responding)
        printf("%u slave(s).\n", ms.slaves_responding);
    if (ms.al_states != master_state.al_states)
        printf("AL states: 0x%02X.\n", ms.al_states);
    if (ms.link_up != master_state.link_up)
        printf("Link is %s.\n", ms.link_up ? "up" : "down");

    master_state = ms;
}

/*****************************************************************************/

void check_slave_config_states(void)
{
    ec_slave_config_state_t s;

    ecrt_slave_config_state(sc_ana_in, &s);

    if (s.al_state != sc_ana_in_state.al_state)
        printf("AnaIn: State 0x%02X.\n", s.al_state);
    if (s.online != sc_ana_in_state.online)
        printf("AnaIn: %s.\n", s.online ? "online" : "offline");
    if (s.operational != sc_ana_in_state.operational)
        printf("AnaIn: %soperational.\n",
                s.operational ? "" : "Not ");

    sc_ana_in_state = s;
}

/*****************************************************************************/

#if SDO_ACCESS
void read_sdo(void)
{
    switch (ecrt_sdo_request_state(sdo)) {
        case EC_REQUEST_UNUSED: // request was not used yet
            ecrt_sdo_request_read(sdo); // trigger first read
            break;
        case EC_REQUEST_BUSY:
            fprintf(stderr, "Still busy...\n");
            break;
        case EC_REQUEST_SUCCESS:
            fprintf(stderr, "SDO value: 0x%04X\n",
                    EC_READ_U16(ecrt_sdo_request_data(sdo)));
            ecrt_sdo_request_read(sdo); // trigger next read
            break;
        case EC_REQUEST_ERROR:
            fprintf(stderr, "Failed to read SDO!\n");
            ecrt_sdo_request_read(sdo); // retry reading
            break;
    }
}
#endif

/****************************************************************************/
unsigned int out1_motor, out2_operation, out3_torque, out4_position, out5_velocity;
bool recvFlag = false;
bool changeDataFlag = false;
bool initFlag = true;
struct timespec tsSend;
struct timespec tsRecv;
long timeMin = LONG_MAX;
long timeMax = LONG_MIN;
long timeSum = 0;
int counter = 0;
 
void cyclic_task()
{	
	// receive process data
	ecrt_master_receive(master);
	ecrt_domain_process(domain1);
	// check process data state (optional)
	check_domain1_state();
	// read process data
	unsigned int in1_motor = EC_READ_U16(domain1_pd + off_in_pdo1); // motor
	unsigned int in2_operation = EC_READ_U8(domain1_pd + off_in_pdo2);  // operation
	unsigned int in3_position = EC_READ_U32(domain1_pd + off_in_pdo3); // position
	unsigned int in4_velocity = EC_READ_U32(domain1_pd + off_in_pdo4); // velocity
	unsigned int in5_torque = EC_READ_U16(domain1_pd + off_in_pdo5); // torque
	if ((in1_motor == out1_motor) && (in2_operation == out2_operation) &&\
		(in3_position == out4_position) && (in4_velocity == out5_velocity) &&\
		(in5_torque == out3_torque))
	{
		clock_gettime(CLOCK_MONOTONIC, &tsRecv);
		if((tsRecv.tv_sec - tsSend.tv_sec) <= DIFFMAX) {
			long timeDiff = NSEC_PER_SEC * (tsRecv.tv_sec - tsSend.tv_sec) + 
					(tsRecv.tv_nsec - tsSend.tv_nsec);
			if(timeDiff < timeMin)
				timeMin = timeDiff;
			if(timeDiff > timeMax)
				timeMax = timeDiff;
			timeSum += timeDiff;
			counter++;
			printf("%d Max: %ld, Min: %ld (ns)\n", 
				counter, timeMax, timeMin);
		}
		recvFlag = true;
		changeDataFlag = !changeDataFlag;
	}
	out1_motor = (unsigned int)(changeDataFlag ? HEX88CC : HEXCC88);
	out2_operation = (unsigned int)(changeDataFlag ? HEXF1 : HEX1F);
	out3_torque = (unsigned int)(changeDataFlag ? HEX2244 : HEX4422);
	out4_position = (unsigned int)(changeDataFlag ? HEX4D4D5E5E : HEX5E5E4D4D);
	out5_velocity = (unsigned int)(changeDataFlag ? HEX1D1D2E2E : HEX2E2E1D1D);
	EC_WRITE_U16(domain1_pd + off_out_pdo1, (changeDataFlag ? HEX88CC : HEXCC88) & 0xffff);
	EC_WRITE_U8(domain1_pd + off_out_pdo2, (changeDataFlag ? HEXF1 : HEX1F));
	EC_WRITE_U16(domain1_pd + off_out_pdo3, (changeDataFlag ? HEX2244 : HEX4422) & 0xffff);
	EC_WRITE_U32(domain1_pd + off_out_pdo4, (changeDataFlag ? HEX4D4D5E5E : HEX5E5E4D4D));
	EC_WRITE_U32(domain1_pd + off_out_pdo5, (changeDataFlag ? HEX1D1D2E2E : HEX2E2E1D1D));
	//printf("SEND: %u %u %u %u %u\n", out1_motor, out2_operation, out3_torque, out4_position, out5_velocity);
	// send process data
	ecrt_domain_queue(domain1);
	ecrt_master_send(master);
	if(recvFlag || initFlag)
	{
		// send time
		clock_gettime(CLOCK_MONOTONIC, &tsSend);
		recvFlag = false;
		initFlag = false;
	}
}

/****************************************************************************/

void signal_handler(int signum) {
    switch (signum) {
        case SIGALRM:
            sig_alarms++;
            break;
    }
}

/****************************************************************************/

int main(int argc, char **argv)
{
    ec_slave_config_t *sc;
    struct sigaction sa;
    struct itimerval tv;

    master = ecrt_request_master(0);
    if (!master)
        return -1;

    domain1 = ecrt_master_create_domain(master);
    if (!domain1)
        return -1;

    if (!(sc_ana_in = ecrt_master_slave_config(
                    master, SomaSlavePos, SomaEtherCAT))) {
        fprintf(stderr, "Failed to get slave configuration.\n");
        return -1;
    }

#if SDO_ACCESS
    fprintf(stderr, "Creating SDO requests...\n");
    if (!(sdo = ecrt_slave_config_create_sdo_request(sc_ana_in, 0x3102, 2, 2))) {
        fprintf(stderr, "Failed to create SDO request.\n");
        return -1;
    }
    ecrt_sdo_request_timeout(sdo, 500); // ms
#endif

#if CONFIGURE_PDOS
    printf("Configuring PDOs...\n");
    if (ecrt_slave_config_pdos(sc_ana_in, EC_END, slave_0_syncs)) {
        fprintf(stderr, "Failed to configure PDOs.\n");
        return -1;
    }

    if (!(sc = ecrt_master_slave_config(
                    master, SomaSlavePos, SomaEtherCAT))) {
        fprintf(stderr, "Failed to get slave configuration.\n");
        return -1;
    }
#endif

    // Create configuration for bus coupler
    sc = ecrt_master_slave_config(master, SomaSlavePos, SomaEtherCAT);
    if (!sc)
        return -1;

    if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs)) {
        fprintf(stderr, "PDO entry registration failed!\n");
        return -1;
    }

    printf("Activating master...\n");
    if (ecrt_master_activate(master))
        return -1;

    if (!(domain1_pd = ecrt_domain_data(domain1))) {
        return -1;
    }

#if PRIORITY
    pid_t pid = getpid();
    if (setpriority(PRIO_PROCESS, pid, -19))
        fprintf(stderr, "Warning: Failed to set priority: %s\n",
                strerror(errno));
#endif

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, 0)) {
        fprintf(stderr, "Failed to install signal handler!\n");
        return -1;
    }

    printf("Starting timer...\n");
    tv.it_interval.tv_sec = 0;
    tv.it_interval.tv_usec = 1000000 / FREQUENCY;
    tv.it_value.tv_sec = 0;
    tv.it_value.tv_usec = 1000;
    if (setitimer(ITIMER_REAL, &tv, NULL)) {
        fprintf(stderr, "Failed to start timer: %s\n", strerror(errno));
        return 1;
    }

    printf("Started.\n");
    while (1) {
        pause();

#if 0
        struct timeval t;
        gettimeofday(&t, NULL);
        printf("%u.%06u\n", t.tv_sec, t.tv_usec);
#endif

        while (sig_alarms != user_alarms) {
            cyclic_task();
            user_alarms++;
        }
    }

    return 0;
}

/****************************************************************************/
