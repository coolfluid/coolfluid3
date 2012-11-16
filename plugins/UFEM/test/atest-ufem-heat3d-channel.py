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
generator = domain.create_component('generator', 'cf3.mesh.BlockMesh.ChannelGenerator')
generator.options().set('mesh', cf.URI('//HotModel/Domain/mesh'))
generator.options().set('x_segments', 64)
generator.options().set('cell_overlap', 1)
generator.execute()

hc.options().set('regions', [domain.access_component('mesh/topology').uri()])

# Boundary conditions
bc = hc.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'left', variable_name = 'Temperature')
bc.add_constant_bc(region_name = 'right', variable_name = 'Temperature')
bc.get_child('BCleftTemperature').options().set('value', 10)
bc.get_child('BCrightTemperature').options().set('value', 35)

# run the simulation
model.simulate()

# print timings
model.print_timing_tree()

# Write result
domain.write_mesh(cf.URI('atest-ufem-heat3d-channel_output.pvtu'))
