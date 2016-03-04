import sys
import coolfluid as cf

# Global configuration
cf.Core.environment().options().set('log_level', 4)

# setup a model
model = cf.Core.root().create_component('HotModel', 'cf3.solver.Model')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.KEpsilonPhysics')

if str(round(physics.yplus,2)) != '11.25':
    raise(RuntimeError("Error in standard yplus"))
