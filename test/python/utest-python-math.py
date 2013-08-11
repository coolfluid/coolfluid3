import coolfluid as cf

a = 0.2345678901234567890
b = 1.2345678901234567890

test = cf.Core.root().create_component('Test', 'cf3.python.TestSignals')

# test setting and reading a property
test.real = a
if not abs(cf.math.float_distance(a, test.real)) < 1:
  raise Exception('test.real ({r}) differs from expected value {a}'.format(r = test.real, a = a))
else:
  print 'distance between {a} and {r} is {d}'.format(a = repr(a), r = repr(test.real), d = cf.math.float_distance(a, test.real))

# test setting through a signal
test.set_real(b)
if not abs(cf.math.float_distance(b, test.real)) < 1:
  raise Exception('test.real ({r}) differs from expected value {b}'.format(r = test.real, b = b))
else:
  print 'distance between {a} and {r} is {d}'.format(a = repr(b), r = repr(test.real), d = cf.math.float_distance(b, test.real))
