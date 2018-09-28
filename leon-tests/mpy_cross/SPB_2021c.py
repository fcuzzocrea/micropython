# compiler should reject this

def foo():
    while 1:
        try:
            pass
        finally:
            try:
                pass
            finally:
                return
