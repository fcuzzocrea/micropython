/*
 * This file is part of the MicroPython port to MPFS platforms
 *
 * Based on LEON port originally made by George Robotics
 *
 * Copyright (c) 2023 Francescodario Cuzzocrea <bosconovic@gmail.com>
 */

/*
 * This file implements the dummy temperature reading task.
 */

#include <stdio.h>
#include <math.h>
#include <rtems.h>
#include "obcp.h"
#include "mpfs-common/moddatapool.h"

rtems_task obcp_task_temp(rtems_task_argument task_arg) {
    printf("TEMP task started\n");

    datapool_result_t dp_res;
    datapool_t dp;
    dp_res = datapool_ident("DATAPOOL", &dp);

    if (dp_res != DATAPOOL_OK) {
        printf("DATAPOOL error: %d\n", dp_res);
        return;
    }

    // loop forever, changing the temperature in the datapool
    for (;;) {
        double t = get_time();
        double temps[N_TEMP];
        for (int i = 0; i < N_TEMP; ++i) {
            temps[i] = 273 + 0.4 * i * sin(t); // add some variation based on the time
        }
        dp_res = datapool_set_buf(dp, K_DP_TEMP_VAL_30, temps, sizeof(temps));
        if (dp_res != DATAPOOL_OK) {
            printf("DATAPOOL error: %d\n", dp_res);
            return;
        }

        rtems_task_wake_after(20);
    }
}
