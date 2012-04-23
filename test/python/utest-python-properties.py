from coolfluid import *

root = Core.root()

print root.properties()['brief']

norm = root.create_component('norm', 'cf3.solver.actions.ComputeLNorm')
cf_check_equal(norm.properties()['norm'],0,'Norm was not zero')

