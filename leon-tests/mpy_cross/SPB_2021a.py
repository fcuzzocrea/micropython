# compiler should reject this

while 1:
    try:
        pass
    finally:
        break
