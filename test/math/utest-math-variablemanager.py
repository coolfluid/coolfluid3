import coolfluid as cf

manager = cf.root.create_component('varmanager', 'cf3.math.VariableManager')

descriptor = manager.create_descriptor(name='solution', description='a, b[v], c[t]')
descriptor.dimension = 2

if descriptor.properties()['variables_description'] != 'a[scalar],b[vector],c[tensor]':
  raise(Exception('Error: unexpected description value'))
