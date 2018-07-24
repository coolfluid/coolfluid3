import coolfluid as cf
import sys

def my_filter(component):
  return component.name() == 'testpass'

cf.Core.environment().options().set('log_level', 4)

root = cf.Core.root()

test_pass = root.create_component('testpass', 'cf3.common.Group')
test_fail = root.create_component('testfail', 'cf3.common.Group')
filter = root.create_component('filter', 'cf3.python.ComponentFilterPython')

filter.set_callable(my_filter)
if not filter(test_pass):
  raise Exception('Test Failed')
print filter(test_fail)
