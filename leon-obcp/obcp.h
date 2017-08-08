/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#pragma once

// configuration for the datapool
#define DATAPOOL_HEAP_SIZE (8 * 1024)

// configuration for the MicroPython VM worker tasks
#define VM_WORKER_NUM_TASKS (4)
#define VM_WORKER_TASK_ATTRIBUTES (RTEMS_APPLICATION_TASK | RTEMS_FLOATING_POINT)
#define VM_WORKER_STACK_SIZE (RTEMS_MINIMUM_STACK_SIZE * 3)
#define VM_WORKER_HEAP_SIZE (48 * 1024)

// configuration for the native tasks (TM, TC, TEMP, GND)
#define NATIVE_TASK_ATTRIBUTES (RTEMS_APPLICATION_TASK | RTEMS_FLOATING_POINT)
#define NATIVE_TASK_STACK_SIZE (RTEMS_MINIMUM_STACK_SIZE)

rtems_task obcp_task_mp_worker(rtems_task_argument task_index);
rtems_task obcp_task_tm(rtems_task_argument task_arg);
rtems_task obcp_task_tc(rtems_task_argument task_arg);
rtems_task obcp_task_temp(rtems_task_argument task_arg);
rtems_task obcp_task_ground(rtems_task_argument task_arg);

// number of sensors
#define N_VOLT (10)
#define N_CURR (10)
#define N_TEMP (30)

// telecommand constants
#define K_TC_MAX_LEN            (64)
#define K_TC_CMD_MPY_LOAD       (16)
#define K_TC_CMD_MPY_START      (17)
#define K_TC_CMD_SET_VOLT_THRESH (18)
#define K_TC_CMD_SET_CURR_THRESH (19)

// telemetry constants
#define K_TM_MAX_LEN            (32)
#define K_TM_VOLT_RANGE         (140)
#define K_TM_CURR_RANGE         (141)
#define K_TM_TEMP_RANGE         (142)

// datapool constants
#define K_DP_VOLT_THRESH_10     (100)
#define K_DP_CURR_THRESH_10     (101)
#define K_DP_TEMP_VAL_30        (102)
#define K_DP_TEMP_THRESH_30     (103)

// TC
typedef union {
    struct {
        uint8_t len; // in bytes, including cmd entry
        uint8_t cmd;
        uint8_t data[K_TC_MAX_LEN - 2];
    } generic;
    struct {
        uint8_t len;
        uint8_t cmd;
        uint8_t vm_id;
        uint8_t _pad0;
        uint16_t data_off;
        uint16_t data_len;
        uint8_t data[K_TC_MAX_LEN - 8];
    } mpy_load;
    struct {
        uint8_t len;
        uint8_t cmd;
        uint8_t vm_id;
        uint8_t _pad0;
        uint16_t mpy_len;
        uint8_t hash[32];
    } mpy_start;
    struct {
        uint8_t len;
        uint8_t cmd;
        uint8_t channel;
        uint8_t _pad0[5];
        double low, high;
    } thresh;
} TC_t;

// TM
typedef struct {
    unsigned char len;
    unsigned char data[K_TM_MAX_LEN - 1];
} TM_t;

void set_start_time(void);
double get_time(void);

// Accessor to get the state for each VM worker
struct _mp_state_ctx_t;
extern struct _mp_state_ctx_t *get_vm_worker_ctx(int id);
