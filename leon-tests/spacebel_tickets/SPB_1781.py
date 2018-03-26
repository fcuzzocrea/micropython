# round would overflow internally with large second arg

print('%e' % round(1.4e-300, 300))
print('%e' % round(1.4e-315, 315))
print('%e' % round(1.4e-315, 400))

print('%e' % round(1.4e+300, -300))
print('%e' % round(1.4e+305, -320))
print('%e' % round(1.4e+305, -330))
