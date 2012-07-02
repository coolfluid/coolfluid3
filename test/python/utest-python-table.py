from coolfluid import *

root = Core.root()
env = Core.environment()

env.options().set('assertion_backtrace', False)
env.options().set('exception_backtrace', False)
env.options().set('regist_signal_handlers', False)
env.options().set('exception_log_level', 0)
env.options().set('log_level', 4)
env.options().set('exception_outputs', False)

table = root.create_component("table", "cf3.common.Table<unsigned>")

cf_check_equal(len(table),0,'Created table was not size 0')

table.set_row_size(2)
table.resize(10)

cf_check_equal(len(table),10,'Incorrect table size')

cf_check_equal(len(table[0]),2,'Incorrect table column count')

table[0] = [1, 2]
print 'First table row is', table[0]

cf_check(table[0][0] == 1 and table[0][1] == 2,'First table row is incorrect')

table[1] = table[0]
print table[1]

cf_check(table[1][0] == 1 and table[1][1] == 2,'Second table row is incorrect')

table[0][0] += 1
print table[0]
cf_check_equal(table[0][0],2,'Table increment failed')

print 'Full table:'
print table
