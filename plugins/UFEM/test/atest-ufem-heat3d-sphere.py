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
model = root.create_component('HotModel', 'cf3.Solver.CModel')
model.setup(solver_builder = 'cf3.UFEM.HeatConductionSteady', physics_builder = 'cf3.Physics.DynamicModel')
solver = model.get_child('HeatConductionSteady')
domain = model.get_child('Domain')

# Generate a channel mesh
domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

# lss setup
lss = model.create_component('LSS', 'cf3.math.LSS.System')
lss.configure_option('solver', 'Trilinos')
solver.configure_option('lss', lss.uri())
lss.get_child('Matrix').configure_option('settings_file', sys.argv[2])

# Boundary conditions
bc = solver.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'inner', variable_name = 'Temperature')
bc.add_constant_bc(region_name = 'outer', variable_name = 'Temperature')
bc.get_child('BCinnerTemperature').configure_option('value', 10)
bc.get_child('BCouterTemperature').configure_option('value', 35)

# run the simulation
model.simulate()

# print timings
model.print_timing_tree()

# Write result
domain.create_component('VTKwriter', 'cf3.mesh.VTKXML.Writer')
domain.write_mesh(cf.URI('atest-ufem-heat3d-sphere_output.pvtu'))
