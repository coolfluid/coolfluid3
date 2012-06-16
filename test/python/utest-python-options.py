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
test_option(opts01, 'generic_component_vector', [opts01, root])
test_option(opts01, 'group_component_vector', [root, root])

print '########################### Testing global config ##########################'

# test configure signal
testgrp = opts01.create_component('testgrp', 'cf3.common.Group')
testgen = opts01.create_component('testgen', 'cf3.common.Component')
opts01.configure(string = 'global_config', real = 4, int = 5, uint = 6, bool = False, uri = testgen.uri(), generic_component = testgen, group_component = testgrp, string_vector = ['d', 'e'], int_vector = [3, 4, 5], uint_vector = [3, 4, 5], real_vector = [6, 7.2, 8.3], bool_vector = [True, False, True], generic_component_vector = [opts01, testgen], group_component_vector = [testgrp, root])
check_option(opts01, 'string', 'global_config')
check_option(opts01, 'real', 4.)
check_option(opts01, 'int', 5)
check_option(opts01, 'uint', 6)
check_option(opts01, 'bool', False)
check_option(opts01, 'uri', testgen.uri())
check_option(opts01, 'generic_component', testgen)
check_option(opts01, 'group_component', testgrp)
check_option(opts01, 'string_vector', ['d', 'e'])
check_option(opts01, 'int_vector', [3, 4, 5])
check_option(opts01, 'uint_vector', [3, 4, 5])
check_option(opts01, 'real_vector', [6, 7.2, 8.3])
check_option(opts01, 'bool_vector', [True, False, True])
check_option(opts01, 'generic_component_vector', [opts01, testgen])
check_option(opts01, 'group_component_vector', [testgrp, root])

print '########################### Checking basic access ##########################'

if opts01.string != 'global_config':
  raise Exception('Bad option value')

opts01.string = 'easy_set'
check_option(opts01, 'string', 'easy_set')
if opts01.string != 'easy_set':
  raise Exception('Bad option value')

opts01.const_component = root
check_option(opts01, 'const_component', root)

print '########################### Checking access through options attribute ##########################'

opts01.options.generic_component_vector = [testgen, opts01]
opts01.options['string_vector'] = ['f', 'g']
check_option(opts01, 'generic_component_vector', [testgen, opts01])
check_option(opts01, 'string_vector', ['f', 'g'])

print '############## Access methods demo #############'

print opts01.string # Only valid for basic options
print opts01.options.string
print opts01.options['string']
print opts01.options().string
print opts01.options()['string']