import sys
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global confifuration
env.options().configure_option('assertion_throws', False)
env.options().configure_option('assertion_backtrace', False)
env.options().configure_option('exception_backtrace', False)
env.options().configure_option('regist_signal_handlers', False)
env.options().configure_option('log_level', 1)

# setup a model
model = cf.Core.root().create_component('HotModel', 'cf3.solver.Model')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')
hc = solver.add_direct_solver('cf3.UFEM.HeatConductionSteady')

# Generate a channel mesh
generator = domain.create_component('generator', 'cf3.mesh.BlockMesh.ChannelGenerator')
generator.options().configure_option('mesh', cf.URI('//HotModel/Domain/mesh'))
generator.options().configure_option('x_segments', 64)
generator.options().configure_option('cell_overlap', 1)
generator.execute()

# lss setup
lss = hc.create_lss('cf3.math.LSS.TrilinosFEVbrMatrix')
lss.get_child('Matrix').options().configure_option('settings_file', sys.argv[1]);

# Boundary conditions
bc = hc.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'left', variable_name = 'Temperature')
bc.add_constant_bc(region_name = 'right', variable_name = 'Temperature')
bc.get_child('BCleftTemperature').options().configure_option('value', 10)
bc.get_child('BCrightTemperature').options().configure_option('value', 35)

# run the simulation
model.simulate()

# print timings
model.print_timing_tree()

# Write result
domain.write_mesh(cf.URI('atest-ufem-heat3d-channel_output.pvtu'))
