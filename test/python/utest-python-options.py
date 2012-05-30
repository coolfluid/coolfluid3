import coolfluid as cf
import sys

root = cf.Core.root()

cf.Core.environment().options().set('log_level', 4)

def check_option(comp, optname, value):
  print 'option', optname, 'for component', comp.uri(), 'has value', comp.options()[optname], 'and should be', value
  if comp.options()[optname] != value:
    raise Exception('Bad option value')

def test_option(comp, optname, value):
  comp.options().set(optname, value)
  check_option(comp, optname, value)

opts01 = root.create_component('opts01', 'cf3.python.TestAllOptions')

test_option(opts01, 'string', 'teststring')
test_option(opts01, 'real', 2.)
test_option(opts01, 'real', 2)
test_option(opts01, 'int', 3)
test_option(opts01, 'uint', 4)
test_option(opts01, 'bool', True)
test_option(opts01, 'uri', opts01.uri())
test_option(opts01, 'generic_component', opts01)
test_option(opts01, 'group_component', root)
test_option(opts01, 'string_vector', ['a', 'b', 'c'])
test_option(opts01, 'int_vector', [0, 1, 2])
test_option(opts01, 'uint_vector', [0, 1, 2])
test_option(opts01, 'real_vector', [0, 1.2, 2.9])
test_option(opts01, 'real_vector', [0, 1, 2])
test_option(opts01, 'bool_vector', [False, True, False])

print '########################### Testing global config ##########################'

# test configure signal
#opts01.configure(string = 'global_config', int_vector = [3, 4, 5], real_vector = [6, 7, 8], uri = root.uri(), generic_component = root, group_component = root)
#opts01.configure(string = 'global_config', uri = root.uri(), generic_component = root, group_component = root)
#opts01.configure(string_vector = ['d', 'e'])
opts01.configure(string = 'global_config')
check_option(opts01, 'string', 'global_config')
#check_option(opts01, 'int_vector', [3, 4, 5])
#check_option(opts01, 'real_vector', [6, 7, 8])
#check_option(opts01, 'uri', root.uri())
#check_option(opts01, 'string_vector', ['d', 'e'])