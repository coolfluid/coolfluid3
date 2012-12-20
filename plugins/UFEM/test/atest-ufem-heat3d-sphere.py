import sys
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global confifuration
env.options().set('assertion_throws', False)
env.options().set('assertion_backtrace', False)
env.options().set('exception_backtrace', False)
env.options().set('regist_signal_handlers', False)
env.options().set('log_level', 1)

# setup a model
model = cf.Core.root().create_component('HotModel', 'cf3.solver.Model')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')
hc = solver.add_direct_solver('cf3.UFEM.HeatConductionSteady')

# Generate a channel mesh
mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

hc.options().set('regions', [mesh.access_component('topology').uri()])

# Boundary conditions
bc = hc.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'inner', variable_name = 'Temperature')
bc.add_constant_bc(region_name = 'outer', variable_name = 'Temperature')
bc.get_child('BCinnerTemperature').options().set('value', 10)
bc.get_child('BCouterTemperature').options().set('value', 35)

# run the simulation
model.simulate()

# print timings
model.print_timing_tree()

# Write result
domain.write_mesh(cf.URI('atest-ufem-heat3d-sphere_output.pvtu'))
