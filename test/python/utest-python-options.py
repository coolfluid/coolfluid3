import coolfluid as cf
import sys

root = cf.Core.root()

cf.Core.environment().options().configure_option('log_level', 4)

def test_option(comp, optname, value):
  comp.options().configure_option(optname, value)
  print 'option', optname, 'for component', comp.uri(), 'has value', comp.options()[optname], 'and was set using', value
  if comp.options()[optname] != value:
    raise Exception('Bad option value')

opts01 = root.create_component('opts01', 'cf3.python.TestAllOptions')

test_option(opts01, 'string', 'teststring')
test_option(opts01, 'real', 2.)
test_option(opts01, 'int', 3)
test_option(opts01, 'uint', 4)
test_option(opts01, 'bool', True)
test_option(opts01, 'uri', opts01.uri())
test_option(opts01, 'generic_component', opts01)
test_option(opts01, 'group_component', root)
test_option(opts01, 'string_vector', ['a', 'b', 'c'])
test_option(opts01, 'int_vector', [0, 1, 2])
test_option(opts01, 'uint_vector', [0, 1, 2])
