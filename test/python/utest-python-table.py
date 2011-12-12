import coolfluid as cf
import sys

root = cf.Core.root()
env = cf.Core.environment()

env.options().configure_option('assertion_backtrace', False)
env.options().configure_option('exception_backtrace', False)
env.options().configure_option('regist_signal_handlers', False)
env.options().configure_option('exception_log_level', 0)
env.options().configure_option('log_level', 4)
env.options().configure_option('exception_outputs', False)

table = root.create_component("table", "cf3.common.Table<unsigned>")

if len(table) != 0:
  raise Exception('Created table was not size 0')

table.set_row_size(2)
table.resize(10)

if len(table) != 10:
  raise Exception('Incorrect table size')

if len(table[0]) != 2:
  raise Exception('Incorrect table column count')

table[0] = [1, 2]
print 'First table row is', table[0]

if table[0][0] != 1 or table[0][1] != 2:
  raise Exception('First table row is incorrect')

table[1] = table[0]
print table[1]
if table[1][0] != 1 or table[1][1] != 2:
  raise Exception('Second table row is incorrect')

table[0][0] += 1
print table[0]
if table[0][0] != 2:
  raise Exception('Table increment failed')

print 'Full table:'
print table