import coolfluid as cf
import sys

cf.Core.environment().options().set('log_level', 4)

root = cf.Core.root()

parent = root.create_component('parent', 'cf3.common.Group')

parent.create_component('child01', 'cf3.common.Group').mark_basic()

child02 = parent.create_component('child02', 'cf3.common.Group')
child02.mark_basic()

print parent.child01
print parent.child02

parent.mark_basic()

parent.child02.create_component('child03', 'cf3.common.Group').mark_basic()

print parent.child02.child03