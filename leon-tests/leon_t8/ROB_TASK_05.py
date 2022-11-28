# Test for MicroPython port to LEON
# Test ID: ROB_TASK_05
# Test description: Do heavy processing (Conway's Game Of Life) in multiple tasks,
#                   running on all available CPUs.  The tasks must synchronise and
#                   coordinate to do the computation correctly.

import sys
import time
import rtems
import datapool

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
            while True:
                try:
                    self.sem = rtems.sem.ident("LOCK")
                    break
                except OSError:
                    pass

    def delete(self):
        self.sem.delete()

    def __enter__(self):
        self.sem.obtain()

    def __exit__(self, a, b, c):
        self.sem.release()


# Class providing a double-buffered grid for Conway's Game Of Life.
class ConwayGrid:
    RENDER_CHARS = (".", "#")

    def __init__(self, datapool_grid):
        self.grid = datapool_grid
        self.width = self.grid.get_u32(-1)
        self.height = self.grid.get_u32(-2)
        self.buf = bytearray(self.width)

    # Prints the current buffer.
    def print(self):
        for y in range(self.height):
            print("".join(self.RENDER_CHARS[self.get(x, y)] for x in range(self.width)))

    # Copies the second buffer to the first.
    def copy_buffer(self):
        with lock:
            for y in range(self.height):
                self.grid.get_buf(y + self.height, self.buf)
                self.grid.set_buf(y, self.buf)

    # Clears the second buffer.
    def clear(self):
        for y in range(self.height):
            for x in range(self.width):
                self.set(x, y, 0)

    # Adds a block to the second buffer.
    def add_block(self, x, y):
        # Block pattern:
        #   ##
        #   ##
        self.set(x + 0, y + 0, 1)
        self.set(x + 1, y + 0, 1)
        self.set(x + 0, y + 1, 1)
        self.set(x + 1, y + 1, 1)

    # Adds a blinker to the second buffer.
    def add_blinker(self, x, y):
        # Blinker pattern:
        #   ###
        self.set(x + 0, y + 0, 1)
        self.set(x + 1, y + 0, 1)
        self.set(x + 2, y + 0, 1)

    # Adds a glider to the second buffer.
    def add_glider(self, x, y):
        # Glider pattern:
        #   .#.
        #   ..#
        #   ###
        self.set(x + 1, y + 0, 1)
        self.set(x + 2, y + 1, 1)
        self.set(x + 0, y + 2, 1)
        self.set(x + 1, y + 2, 1)
        self.set(x + 2, y + 2, 1)

    # Gets from the first buffer.
    def get(self, x, y):
        self.grid.get_buf(y % self.height, self.buf)
        return self.buf[x % self.width]

    # Sets to the second buffer.
    def set(self, x, y, value):
        with lock:
            self.grid.get_buf(y % self.height + self.height, self.buf)
            self.buf[x % self.width] = value
            self.grid.set_buf(y % self.height + self.height, self.buf)

    # Does one step in the Game Of Life, updating the second buffer.
    def step(self, x_start, y_start, width, height):
        for y in range(y_start, y_start + height):
            for x in range(x_start, x_start + width):
                # Count number of neighbours.
                num_neighbours = (
                    self.get(x - 1, y - 1)
                    + self.get(x, y - 1)
                    + self.get(x + 1, y - 1)
                    + self.get(x - 1, y)
                    + self.get(x + 1, y)
                    + self.get(x + 1, y + 1)
                    + self.get(x, y + 1)
                    + self.get(x - 1, y + 1)
                )

                # Check if the centre cell is alive or not.
                me = self.get(x, y)

                # Apply the rules of life.
                if me and not (2 <= num_neighbours <= 3):
                    self.set(x, y, 0)  # not enough, or too many neighbours: cell dies
                elif not me and num_neighbours == 3:
                    self.set(x, y, 1)  # 3 neighbours around empty cell: cell is born


# Class representing atomic counters.
class Counter:
    SEQUENCE = 0
    FINISH = 1

    def __init__(self, datapool_counter):
        self.datapool = datapool_counter

    def set(self, counter, value):
        self.datapool.set_u32(counter, value)

    def add(self, counter, value):
        with lock:
            self.datapool.set_u32(counter, self.datapool.get_u32(counter) + value)

    def wait_until_equal(self, counter, value):
        timeout = 0
        while self.datapool.get_u32(counter) != value:
            # Wait with minimal latency.
            time.sleep(0.001)
            # Abort if waiting takes too long.
            timeout += 1
            if timeout > 10000:
                raise Exception("Counter.wait_until_equal: timeout")


def conway_sync_task(width, height, steps):
    global lock

    datapool_counter = datapool.create("COUNT")
    datapool_counter.set_u32(Counter.SEQUENCE, 0)
    datapool_counter.set_u32(Counter.FINISH, 0)
    counter = Counter(datapool_counter)

    datapool_grid = datapool.create("GRID")
    datapool_grid.set_u32(-1, width)
    datapool_grid.set_u32(-2, height)
    for y in range(2 * height):
        datapool_grid.set_buf(y, bytearray(width))
    conway_grid = ConwayGrid(datapool_grid)

    # Creating the lock object will ungate the worker tasks.
    lock = Lock()

    # Set up the initial Conway gride
    conway_grid.clear()
    conway_grid.add_glider(0, 0)
    conway_grid.add_block(10, 1)
    conway_grid.add_blinker(15, 2)
    conway_grid.add_blinker(10, 5)
    conway_grid.copy_buffer()
    conway_grid.print()

    # Loop through each iteration step.
    for step in range(steps):
        # Start the worker tasks computing this step.
        counter.set(Counter.FINISH, 0)
        counter.set(Counter.SEQUENCE, 1)
        counter.wait_until_equal(Counter.FINISH, NUM_TASKS - 1)

        # Synchronise all the worker tasks after they finish the step.
        counter.set(Counter.FINISH, 0)
        counter.set(Counter.SEQUENCE, 2)
        counter.wait_until_equal(Counter.FINISH, NUM_TASKS - 1)

        # Copy the new grid to the current one.
        conway_grid.copy_buffer()

        # Print the current grid.
        print("step", step + 1)
        conway_grid.print()

    # Clean up the RTEMS primitives.
    lock.delete()


def conway_worker_task(x_start, y_start, width, height, steps):
    global lock

    # Once this lock is created the sync task has finished initialisation.
    lock = Lock()

    # Create the counter and Conway grid.
    counter = Counter(datapool.ident("COUNT"))
    conway_grid = ConwayGrid(datapool.ident("GRID"))

    # Loop through each iteration step.
    for _ in range(steps):
        # Wait for the sync task to indicate we can start work.
        counter.wait_until_equal(Counter.SEQUENCE, 1)
        conway_grid.step(x_start, y_start, width, height)
        counter.add(Counter.FINISH, 1)

        # Synchronise with the sync task at the end of the step.
        counter.wait_until_equal(Counter.SEQUENCE, 2)
        counter.add(Counter.FINISH, 1)


def main():
    # Set the dimensions of the Conway grid.
    width = NUM_TASKS * 3
    height = (NUM_TASKS - 1) * 1
    steps = 16

    # Run one sync task, and the remaining are worker tasks.
    if rtems.script_id() == 0:
        conway_sync_task(width, height, steps)
    else:
        conway_worker_task(0, (rtems.script_id() - 1) * 2, width, 2, steps)


main()
