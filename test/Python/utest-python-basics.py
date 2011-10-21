import coolfluid as cf
import sys

root = cf.Core.root()
env = cf.Core.environment()

env.configure_option('assertion_backtrace', False)
env.configure_option('exception_backtrace', False)
env.configure_option('regist_signal_handlers', False)
env.configure_option('exception_log_level', 0)
env.configure_option('log_level', 4)
env.configure_option('exception_outputs', False)

journal = root.create_component("journal", "CF.Common.CJournal")
print journal.option_value_str("RecordReplies")

group = root.create_component("group", "CF.Common.CGroup")
print "Before move",journal.uri()
journal.move_component(group.uri())
print "After move",journal.uri()

a = cf.RealVector(2)
a[0] = 1.
a[1] = 2.
print len(a), a[0], a[1]

action_director = root.create_component('director', 'CF.Common.ActionDirector')
action_director.configure_option('action_order', ['a', 'b', 'c'])