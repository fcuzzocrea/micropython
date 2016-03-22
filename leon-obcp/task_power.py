# POWER task

import array
import uctypes
import time
import micropython
import rtems
import datapool
import powersys

# global constants
N_VOLT = 10
N_CURR = 10
K_DP_VOLT_THRESH_10 = 100
K_DP_CURR_THRESH_10 = 101
K_TM_VOLT_RANGE = 140
K_TM_CURR_RANGE = 141
K_TC_CMD_SET_VOLT_THRESH = 18
K_TC_CMD_SET_CURR_THRESH = 19

# global structures (mimic C versions)
tm_descr = {
    'cmd': uctypes.UINT8 | 0,
    'channel': uctypes.UINT8 | 1,
    'low': uctypes.FLOAT64 | 8,
    'high': uctypes.FLOAT64 | 16,
    'value': uctypes.FLOAT64 | 24,
}
pwth_descr = {
    'cmd': uctypes.UINT8 | 1,
    'channel': uctypes.UINT8 | 2,
    'low': uctypes.FLOAT64 | 8,
    'high': uctypes.FLOAT64 | 16,
}

def main():
    print('POWER script started, using VM', rtems.script_id())

    # get the queues
    tm_q = rtems.queue.ident('TLMQ')
    pwth_q = rtems.queue.ident('PWTH')

    # create the arrays to hold the values and thresholds
    volt_val = array.array('d', N_VOLT * [0])
    volt_thresh = array.array('d', N_VOLT * [0, 0])
    curr_val = array.array('d', N_CURR * [0])
    curr_thresh = array.array('d', N_CURR * [0, 0])

    # get initial thresholds from the datapool
    dp = datapool.ident('DATAPOOL')
    dp.get_buf(K_DP_VOLT_THRESH_10, volt_thresh)
    dp.get_buf(K_DP_CURR_THRESH_10, curr_thresh)

    # print initial voltage thresholds
    for i in range(N_VOLT):
        print('POWER: initial volt_thres(%d): [%.2f, %.2f]' % (i, volt_thresh[2 * i], volt_thresh[2 * i + 1]))

    # print initial current thresholds
    for i in range(N_CURR):
        print('POWER: initial curr_thres(%d): [%.2f, %.2f]' % (i, curr_thresh[2 * i], curr_thresh[2 * i + 1]))

    # buffer and struct for TM messages
    tm_buf = bytearray(16)
    tm_st = uctypes.struct(uctypes.addressof(tm_buf), tm_descr)

    # buffer and struct for incoming power-threshold change commands
    pwth_buf = bytearray(64)
    pwth_st = uctypes.struct(uctypes.addressof(pwth_buf), pwth_descr)

    # from now on we are deterministic
    micropython.heap_lock()

    # control loop runs at 10Hz
    while True:
        # read voltages and currents
        powersys.read_voltages_into(volt_val)
        powersys.read_currents_into(curr_val)

        # see if thresholds need updating
        if pwth_q.get_number_pending() != 0:
            pwth_q.receive(pwth_buf, rtems.NO_WAIT)
            if pwth_st.cmd == K_TC_CMD_SET_VOLT_THRESH:
                print('POWER: change voltage threshold:', pwth_st.channel, pwth_st.low, pwth_st.high)
                volt_thresh[2 * pwth_st.channel] = pwth_st.low
                volt_thresh[2 * pwth_st.channel + 1] = pwth_st.high
            elif pwth_st.cmd == K_TC_CMD_SET_CURR_THRESH:
                print('POWER: change current threshold:', pwth_st.channel, pwth_st.low, pwth_st.high)
                curr_thresh[2 * pwth_st.channel] = pwth_st.low
                curr_thresh[2 * pwth_st.channel + 1] = pwth_st.high
            else:
                print('POWER: unknown TC:', pwth_st.cmd)

        # check voltage thresholds
        for i in range(N_VOLT):
            if not volt_thresh[2 * i] <= volt_val[i] <= volt_thresh[2 * i + 1]:
                tm_st.cmd = K_TM_VOLT_RANGE
                tm_st.channel = i
                tm_st.low = volt_thresh[2 * i]
                tm_st.high = volt_thresh[2 * i + 1]
                tm_st.value = volt_val[i]
                tm_q.send(tm_st, rtems.WAIT)

        # check current thresholds
        for i in range(N_CURR):
            if not curr_thresh[2 * i] <= curr_val[i] <= curr_thresh[2 * i + 1]:
                tm_st.cmd = K_TM_CURR_RANGE
                tm_st.channel = i
                tm_st.low = curr_thresh[2 * i]
                tm_st.high = curr_thresh[2 * i + 1]
                tm_st.value = curr_val[i]
                tm_q.send(tm_st, rtems.WAIT)

        # pause
        time.sleep(0.1)

if __name__ == '__main__':
    main()
