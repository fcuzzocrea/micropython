def bm_run(N, M):
    try:
        from utime import ticks_us, ticks_diff
    except ImportError:
        import time

        # LEON specific ticks_us
        ticks_us = lambda: int(time.time() * 1000000)
        ticks_diff = lambda a, b: a - b

    # Pick sensible parameters given N, M
    cur_nm = (0, 0)
    param = None
    for nm, p in bm_params.items():
        if 10 * nm[0] <= 12 * N and nm[1] <= M and nm > cur_nm:
            cur_nm = nm
            param = p
    if param is None:
        print(-1, -1, "SKIP: no matching params")
        return

    # Run and time benchmark
    run, result = bm_setup(param)
    t0 = ticks_us()
    run()
    t1 = ticks_us()
    norm, out = result()

    # LEON specific output: <time> <time-standard-deviation> <score> <score-standard-deviation>
    time = ticks_diff(t1, t0)
    score = 1e6 * norm / time
    print(time, 0, score, 0)
