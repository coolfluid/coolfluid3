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

physics.href = 6.
physics.uref = 10.
physics.z0 = 0.01

if str(round(physics.utau,3)) != '0.625':
    raise(RuntimeError("Error in utau"))
