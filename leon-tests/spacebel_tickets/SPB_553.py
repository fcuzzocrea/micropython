# Missing key in KeyError exception's message

try:
    {}[0]
except KeyError as er:
    print(er)
