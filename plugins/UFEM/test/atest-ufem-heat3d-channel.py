import sys
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global confifuration
env.configure_option('assertion_throws', False)
env.configure_option('assertion_backtrace', False)
env.configure_option('exception_backtrace', False)
env.configure_option('regist_signal_handlers', False)
env.configure_option('log_level', 1)

# setup a model
model = root.create_component('HotModel', 'CF.Solver.CModel')
model.setup(solver_builder = 'CF.UFEM.HeatConductionSteady', physics_builder = 'CF.Physics.DynamicModel')
solver = model.get_child('HeatConductionSteady')
domain = model.get_child('Domain')

# Generate a channel mesh
generator = domain.create_component('generator', 'CF.Mesh.BlockMesh.ChannelGenerator')
generator.configure_option('parent', domain.uri())
generator.configure_option('x_segments', 64)
generator.configure_option('cell_overlap', 1)
generator.execute()

# lss setup
lss = model.create_component('LSS', 'CF.Math.LSS.System')
lss.configure_option('solver', 'Trilinos');
solver.configure_option('lss', lss.uri())
lss.get_child('Matrix').configure_option('settings_file', sys.argv[1]);

# Boundary conditions
bc = solver.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'left', variable_name = 'Temperature')
bc.add_constant_bc(region_name = 'right', variable_name = 'Temperature')
bc.get_child('BCleftTemperature').configure_option('value', 10)
bc.get_child('BCrightTemperature').configure_option('value', 35)

# run the simulation
model.simulate()

# print timings
model.print_timing_tree()

# Write result
domain.create_component('VTKwriter', 'CF.Mesh.VTKXML.CWriter');
domain.write_mesh(cf.URI('atest-ufem-heat3d-channel_output.pvtu'))
