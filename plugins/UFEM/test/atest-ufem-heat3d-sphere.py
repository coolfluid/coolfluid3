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
model = root.create_component('HotModel', 'cf3.solver.Model')
model.setup(solver_builder = 'cf3.UFEM.HeatConductionSteady', physics_builder = 'cf3.physics.DynamicModel')
solver = model.get_child('HeatConductionSteady')
domain = model.get_child('Domain')

# Generate a channel mesh
domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

# lss setup
lss = model.create_component('LSS', 'cf3.math.LSS.System')
lss.options().configure_option('solver', 'Trilinos')
solver.options().configure_option('lss', lss)
lss.get_child('Matrix').options().configure_option('settings_file', sys.argv[2])

# Boundary conditions
bc = solver.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'inner', variable_name = 'Temperature')
bc.add_constant_bc(region_name = 'outer', variable_name = 'Temperature')
bc.get_child('BCinnerTemperature').options().configure_option('value', 10)
bc.get_child('BCouterTemperature').options().configure_option('value', 35)

# run the simulation
model.simulate()

# print timings
model.print_timing_tree()

# Write result
domain.write_mesh(cf.URI('atest-ufem-heat3d-sphere_output.pvtu'))
