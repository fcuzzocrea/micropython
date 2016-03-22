/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

/*
 * This file implements the telemetry (TM) task.
 */

#include <stdio.h>
#include <rtems.h>
#include "obcp.h"

typedef struct {
    uint8_t cmd, channel;
    uint8_t pad[6];
    double low;
    double high;
    double value;
} xyz_t;

rtems_task obcp_task_tm(rtems_task_argument task_arg) {
    printf("TM task started\n");

    rtems_id tlmq_id;
    rtems_name name = rtems_build_name('T', 'L', 'M', 'Q');
    rtems_message_queue_ident(name, 0, &tlmq_id);

    for (;;) {
        xyz_t buf;
        size_t sz = 0;
        rtems_status_code status = rtems_message_queue_receive(tlmq_id, &buf, &sz, RTEMS_WAIT, 0);
        if (status != RTEMS_SUCCESSFUL) {
            printf("[%6.3f] TM message receive failed: %d", get_time(), status);
        } else {
            printf("[%6.3f] TM message: ", get_time());
            switch (buf.cmd) {
                case K_TM_VOLT_RANGE: {
                    printf("voltage(%u) out of range: %.3g outside [%.3g, %.3g]\n", buf.channel, buf.value, buf.low, buf.high);
                    break;
                }
                case K_TM_CURR_RANGE: {
                    printf("current(%u) out of range; %.3g outside [%.3g, %.3g]\n", buf.channel, buf.value, buf.low, buf.high);
                    break;
                }
                case K_TM_TEMP_RANGE: {
                    double value, low, high;
                    memcpy(&low, (uint8_t*)&buf + 2, sizeof(double));
                    memcpy(&high, (uint8_t*)&buf + 10, sizeof(double));
                    memcpy(&value, (uint8_t*)&buf + 18, sizeof(double));
                    printf("temperature(%u) out of range; %.3g outside [%.3g, %.3g]\n", buf.channel, value, low, high);
                    break;
                }
                default:
                    printf("unknown ID %u\n", buf.cmd);
                    break;
            }
        }
    }
}
