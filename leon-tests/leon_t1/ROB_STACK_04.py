# Test for MicroPython port to LEON
# Test ID: ROB_STACK_04

def f(x, y):
    try:
        try:
            try:
                try:
                    try:
                        try:
                            try:
                                try:
                                    try:
                                        try:
                                            if x:
                                                f(x - 1, y + 1)
                                            else:
                                                raise ValueError(x, y)
                                        except Exception as er:
                                            print(repr(er))
                                    except ValueError:
                                        pass
                                except:
                                    pass
                            except:
                                pass
                        except:
                            pass
                    except:
                        pass
                except:
                    pass
            except:
                pass
        except:
            pass
    except:
        pass

for i in range(20):
    print(i, f(i, 0))
