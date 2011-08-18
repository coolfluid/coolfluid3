import coolfluid as cf
import sys

cf.Core.initiate(sys.argv)

root = cf.Core.root()
env = cf.Core.environment()

env.configure_option('assertion_backtrace', False)
env.configure_option('exception_backtrace', False)
env.configure_option('regist_signal_handlers', False)
env.configure_option('exception_log_level', 0)
#env.configure_option('log_level', 4)
env.configure_option('exception_outputs', False)

journal = root.create_component("journal", "CF.Common.CJournal")
print journal.option_value_str("RecordReplies")

group = root.create_component("group", "CF.Common.CGroup")
print "Before move",journal.uri()
journal.move_component(group.uri())
print "After move",journal.uri()