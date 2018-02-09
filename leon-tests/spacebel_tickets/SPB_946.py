# there were problems with parsing floats close to subnormal

print('%.14e' % float('+7E-304'))
print('%.14e' % float('+7E-303'))
print('%.14e' % float('+87575437E-310'))
