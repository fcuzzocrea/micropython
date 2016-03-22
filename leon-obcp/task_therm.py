# THERM task

import array
import struct
import time
import micropython
import rtems
import datapool

# global constants
N_TEMP = 30
K_DP_TEMP_VAL_30 = 102
K_DP_TEMP_THRESH_30 = 103
K_TM_TEMP_RANGE = 142

def main():
    print('THERM script started, using VM', rtems.script_id())

    # get the TM queue
    tm_q = rtems.queue.ident('TLMQ')

    # get the datapool
    dp = datapool.ident('DATAPOOL')

    # create the arrays to hold the values and thresholds
    temp_val = array.array('d', N_TEMP * [0])
    temp_thresh = array.array('d', N_TEMP * [0, 0])

    # get initial thresholds from the datapool
    dp.get_buf(K_DP_TEMP_THRESH_30, temp_thresh)

    # print initial thresholds
    for i in range(N_TEMP):
        print('THERM: initial temp_thres(%d): [%.2f, %.2f]' % (i, temp_thresh[2 * i], temp_thresh[2 * i + 1]))

    # buffer for TM messages (will be populated using struct.pack_into)
    buf = bytearray(26)

    # from now on we are deterministic
    micropython.heap_lock()

    # control loop runs at 1Hz
    while True:
        # get temperatures from the datapool
        dp.get_buf(K_DP_TEMP_VAL_30, temp_val)

        # get thresholds from the datapool
        dp.get_buf(K_DP_TEMP_THRESH_30, temp_thresh)

        # check thresholds
        for i in range(N_TEMP):
            if not temp_thresh[2 * i] <= temp_val[i] <= temp_thresh[2 * i + 1]:
                struct.pack_into('>BBddd', buf, 0, K_TM_TEMP_RANGE, i,
                    temp_thresh[2 * i], temp_thresh[2 * i + 1], temp_val[i])
                tm_q.send(buf, rtems.WAIT)

        # pause
        time.sleep(1)

if __name__ == '__main__':
    main()
