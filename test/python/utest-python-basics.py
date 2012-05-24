import coolfluid as cf
import sys

root = cf.Core.root()
env = cf.Core.environment()

env.options().set('assertion_backtrace', False)
env.options().set('exception_backtrace', False)
env.options().set('regist_signal_handlers', False)
env.options().set('exception_log_level', 0)
env.options().set('log_level', 4)
env.options().set('exception_outputs', False)

print env.options().as_str('log_level')

journal = root.create_component("journal", "cf3.common.Journal")
print journal.options().as_str("RecordReplies")

group = root.create_component("group", "cf3.common.Group")
print "Before move",journal.uri()
journal.move_component(group.uri())
print "After move",journal.uri()

a = cf.RealVector(2)
a[0] = 1.
a[1] = 2.
print len(a), a[0], a[1]

action_director = root.create_component('director', 'cf3.common.ActionDirector')
action_director.options().set('disabled_actions', ['a', 'b', 'c'])

print root.derived_type_name()