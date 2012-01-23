import coolfluid as cf
import sys

root = cf.Core.root()

print root.properties()['brief']

norm = root.create_component('norm', 'cf3.solver.actions.ComputeLNorm')
if norm.properties()['norm'] != 0.:
  raise Exception('Norm was not zero')

