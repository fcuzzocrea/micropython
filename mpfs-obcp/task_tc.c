/*
 * This file is part of the MicroPython port to MPFS platforms
 *
 * Based on LEON port originally made by George Robotics
 *
 * Copyright (c) 2023 Francescodario Cuzzocrea <bosconovic@gmail.com>
 */

/*
 * This file implements the telecommand (TC) task.
 */

#include <stdio.h>
#include <string.h>
#include <rtems.h>
#include "obcp.h"
#include "mpvmmanage.h"

#include "extmod/crypto-algorithms/sha256.h"

// area to put incoming MPY files
#define INCOMING_MPY_MAX_SIZE (16 * 1024)
static uint8_t mpy[VM_WORKER_NUM_TASKS * INCOMING_MPY_MAX_SIZE];

rtems_task obcp_task_tc(rtems_task_argument task_arg) {
    rtems_status_code status;

    printf("TC task started\n");

    // get the id of the TC queue
    rtems_id tcq_id;
    {
        rtems_name name = rtems_build_name('T', 'C', 'Q', ' ');
        status = rtems_message_queue_ident(name, RTEMS_SEARCH_ALL_NODES, &tcq_id);
        if (status != RTEMS_SUCCESSFUL) {
            printf("TC: error getting TC queue: %d\n", status);
            return;
        }
    }

    // get the id of the power threshold queue
    rtems_id ptq_id;
    {
        rtems_name name = rtems_build_name('P', 'W', 'T', 'H');
        status = rtems_message_queue_ident(name, RTEMS_SEARCH_ALL_NODES, &ptq_id);
        if (status != RTEMS_SUCCESSFUL) {
            printf("TC: error getting power threshold queue: %d\n", status);
            return;
        }
    }

    // wait for termination
    for (;;) {
        // Wait until item from queue
	TC_t tc;
	size_t sz;
        status = rtems_message_queue_receive(tcq_id, (void *)&tc, &sz, RTEMS_WAIT, 0);

        if (status != RTEMS_SUCCESSFUL) {
            printf("TCMAN:qrecv error status %d\n", status);
        } else {
            int cmd = tc.generic.cmd;
            switch (cmd) {
                case K_TC_CMD_MPY_LOAD: {
                    unsigned int vm_id = tc.mpy_load.vm_id;
                    unsigned int off = tc.mpy_load.data_off;
                    unsigned int len = tc.mpy_load.data_len;
                    if (off + len > INCOMING_MPY_MAX_SIZE) {
                        printf("[%6.3f] TC data write out of bounds vm=%u off=0x%04x len=%u\n", get_time(), vm_id, off, len);
                    } else {
                        memcpy(&mpy[vm_id * INCOMING_MPY_MAX_SIZE + off], &tc.mpy_load.data[0], len);
                        printf("[%6.3f] TC wrote data to vm=%u off=0x%04x len=%u\n", get_time(), vm_id, off, len);
                    }
                    break;
                }
                case K_TC_CMD_MPY_START: {
                    unsigned int vm_id = tc.mpy_start.vm_id;
                    size_t len = tc.mpy_start.mpy_len;
                    uint8_t *mpy_buf = &mpy[vm_id * INCOMING_MPY_MAX_SIZE];

                    // compute hash
                    CRYAL_SHA256_CTX sha_ctx;
                    uint8_t sha_hash[SHA256_BLOCK_SIZE];
                    sha256_init(&sha_ctx);
                    sha256_update(&sha_ctx, mpy_buf, len);
                    sha256_final(&sha_ctx, sha_hash);

                    // check hash
                    int cmp = memcmp(&tc.mpy_start.hash[0], sha_hash, SHA256_BLOCK_SIZE);

                    if (cmp != 0) {
                        printf("[%6.3f] TC vm=%u len=%zu SHA256 FAIL\n", get_time(), vm_id, len);
                    } else {
                        printf("[%6.3f] TC vm=%u len=%zu SHA256 PASS\n", get_time(), vm_id, len);
                        mp_vm_manager_start_mpy(get_vm_worker_ctx(vm_id), mpy_buf, len);
                    }
                    break;
                }
                case K_TC_CMD_SET_VOLT_THRESH:
                case K_TC_CMD_SET_CURR_THRESH:
                    status = rtems_message_queue_send(ptq_id, &tc, sizeof(tc));
                    if (status != RTEMS_SUCCESSFUL) {
                        printf("could not send on power threshold queue: %d %ld\n", status, sizeof(tc));
                    }
                    break;
                default:
                    printf("[%6.3f] TC unknown ID %u\n", get_time(), cmd);
                    break;
            }
        }
    }
}
