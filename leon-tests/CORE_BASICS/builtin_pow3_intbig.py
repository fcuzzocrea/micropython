# test builtin pow() with integral values
# 3 arg version

try:
    print(pow(3, 4, 7))
except NotImplementedError:
    import sys
    print("SKIP")
    sys.exit()

print(pow(555557, 1000002, 1000003))

# Tests for 3 arg pow with large values

# This value happens to be prime
x = 0xd48a1e2a099b1395895527112937a391d02d4a208bce5d74b281cf35a57362502726f79a632f063a83c0eba66196712d963aa7279ab8a504110a668c0fc38a7983c51e6ee7a85cae87097686ccdc359ee4bbf2c583bce524e3f7836bded1c771a4efcb25c09460a862fc98e18f7303df46aaeb34da46b0c4d61d5cd78350f3edb60e6bc4befa712a849

print(hex(pow(2, 200, x))) # Should not overflow, just 1 << 200
