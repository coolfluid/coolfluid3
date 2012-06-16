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


l = range(0,6)
# Valid child access methods
l[0] = parent.child02 # This only works because child02 is marked as basic
l[1] = parent.get_child('child02')
l[2] = parent.children.child02
l[3] = parent.children['child02']
l[4] = parent.children().child02
l[5] = parent.children()['child02']

for child in l:
  cf.cf_check(child == child02, "Invalid child in list")


# Iteration over children
l = []
for comp_name in parent.children.keys():
  l.append(parent.children[comp_name])
  
for child in l:
  cf.cf_check(child == parent.get_child(child.name()), "Invalid child in list")