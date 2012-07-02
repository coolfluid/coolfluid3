from coolfluid import *

root = Core.root()
env = Core.environment()

env.options().set('assertion_backtrace', False)
env.options().set('exception_backtrace', False)
env.options().set('regist_signal_handlers', False)
env.options().set('exception_log_level', 0)
env.options().set('log_level', 4)
env.options().set('exception_outputs', False)

cf_check(env.log_level == 4,'Failed to set the log level at 4')
#print env.options().value_str('log_level')

journal = root.create_component("journal", "cf3.common.Journal")
cf_check(journal.options.RecordReplies == False,'RecordReplies of the journal is not false')
#print journal.options().value_str("RecordReplies")


group = root.create_component("group", "cf3.common.Group")
before_move = journal.uri()
#print "Before move",journal.uri()
journal.move_component(group.uri())
after_move = journal.uri()
#print "After move",journal.uri()
cf_check(before_move != after_move,'Failed to move the component')

action_director = root.create_component('director', 'cf3.common.ActionDirector')
action_director.options().set('disabled_actions', ['a', 'b', 'c'])

cf_check(root.derived_type_name() == 'cf3.common.Group','Derived type name of Root is not equal to \'cf3.common.Group\'')
#print root.derived_type_name()
