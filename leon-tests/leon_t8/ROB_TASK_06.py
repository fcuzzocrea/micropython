# Test for MicroPython port to LEON
# Test ID: ROB_TASK_06
# Test description: Do processing (compute Fibonacci numbers) in multiple tasks,
#                   running on all available CPUs.  The tasks make heavy use of
#                   RTEMS message queues to perform the calculation.

# The algorithm in this test is a very obscure way of computing Fibonacci numbers.
# It links NUM_TASKS together in a circle, linked via message queues.  Fibonacci
# numbers are sent out the message queues and the next task tries to compute
# higher Fibonacci numbers.  It echos out the original number and possibly a new
# number (if it can compute one).  This continues around the circle and the
# tasks and message queues quickly get flooded with Fibonacci numbers.  Eventually
# the first task stops the process once it receives Fibonacci(100).

import sys
import rtems

NUM_CPU = {"leon2": 1, "leon3": 2, "leon4": 4}[sys.platform]
NUM_TASKS = 8

if NUM_CPU > 1:
    # Run tasks evenly spead out over all CPUs.
    affinity = 1 << (rtems.script_id() % NUM_CPU)

    rtems.task.set_affinity(affinity)
    if rtems.task.get_affinity() != affinity:
        print("Affinity not set:", rtems.task.get_affinity())

# Simple lock with context manager, based on RTEMS semaphore.
class Lock:
    def __init__(self):
        if rtems.script_id() == 0:
            self.sem = rtems.sem.create("LOCK")
        else:
            timeout = 0
            while True:
                try:
                    self.sem = rtems.sem.ident("LOCK")
                    break
                except OSError:
                    pass
                timeout += 1
                if timeout > 2000:
                    raise Exception("Lock.__init__: timeout waiting for LOCK semaphore")

    def delete(self):
        self.sem.delete()

    def __enter__(self):
        self.sem.obtain()

    def __exit__(self, a, b, c):
        self.sem.release()


def get_message(q):
    buf = bytearray(32)
    n = q.receive(buf, timeout=50000)
    message = str(buf[:n], "ascii")
    return message


def parse_message(message):
    a, b = message.split("=")
    n = int(a[2:-1])
    fn = int(b)
    return n, fn


def task_0():
    # Create all queues.
    qs = [
        rtems.queue.create("Q%03u" % i, count=40, max_size=32) for i in range(NUM_TASKS)
    ]

    # Creating the lock object will ungate the worker tasks.
    lock = Lock()

    # Get the incoming and outgoing queues for this task.
    q_incoming = qs[-1]
    q_outgoing = qs[0]

    # Prime the algorithm with the start of the Fibonacci sequence.
    q_outgoing.send("F(0)=0")
    q_outgoing.send("F(1)=1")

    # Run the algorithm.
    max_known = 1
    while True:
        # Get next message on incoming queue.
        message = get_message(q_incoming)

        # Check if the algorithm should stop.
        if message == "STOP":
            with lock:
                print(message)
            break

        # Echo message on outgoing queue.
        q_outgoing.send(message)

        # Parse the message, it's a Fibonacci number.
        n, fn = parse_message(message)

        # Check if it's a new Fibonacci number.
        if n > max_known:
            max_known = n

            # Print out every 10th value.
            if n % 10 == 0:
                with lock:
                    print(message)

            # Stop if a large Fibonacci number is reached.
            if max_known >= 100:
                q_outgoing.send("STOP")

    # Clean up the RTEMS primitives.
    for q in qs:
        q.delete()
    lock.delete()


def task_n():
    # Once this lock is created the main task has finished initialisation.
    lock = Lock()

    # Get the incoming and outgoing queues for this task.
    q_incoming = rtems.queue.ident("Q%03u" % (rtems.script_id() - 1))
    q_outgoing = rtems.queue.ident("Q%03u" % rtems.script_id())

    # Run the algorithm.
    known = {}
    max_known = 0
    while True:
        # Get next message on incoming queue.
        message = get_message(q_incoming)

        # Check if the algorithm should stop.
        if message == "STOP":
            with lock:
                print(message)
            q_outgoing.send(message)
            break

        # Parse the message, it's a Fibonacci number.
        n, fn = parse_message(message)

        # If it's a new value store it and see if a higher value can be generated.
        if n not in known:
            known[n] = fn
            if n - 1 in known:
                # Compute a higher Fibonacci number and send that out.
                q_outgoing.send("F(%u)=%u" % (n + 1, fn + known[n - 1]))

        # Echo the original message if it's close enough to the maximum known
        # Fibonacci number (need this for throttling, otherwise the queues overflow).
        if max(known) - n < 5:
            q_outgoing.send(message)


def main():
    # Run one main task, and the remaining are worker tasks.
    if rtems.script_id() == 0:
        task_0()
    else:
        task_n()


main()
