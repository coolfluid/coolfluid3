from coolfluid import *

root = Core.root()

list = root.create_component("list", "cf3.common.List<bool>")

cf_check_equal(len(list),0,'Created list was not size 0')

list.resize(5)

cf_check_equal(len(list),5,'Incorrect list size')

list[0] = True
print 'First list row is', list[0]

list[1] = False

cf_check(list[0], 'First table row is incorrect')
cf_check(not list[1], 'Second table row is incorrect')

print 'Full list:'
print list
