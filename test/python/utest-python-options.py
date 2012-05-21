import coolfluid as cf
import sys

root = cf.Core.root()

def test_option(comp, optname, value):
  comp.options().configure_option(optname, value)
  print 'option', optname, 'for component', comp.uri(), 'has value', comp.options().value_str(optname), 'and was set using', str(value)
  if comp.options().value_str(optname) != str(value):
    raise Exception('Bad option value')

opts01 = root.create_component('opts01', 'cf3.python.TestAllOptions')

test_option(opts01, 'string', 'teststring')
test_option(opts01, 'real', 2.)

