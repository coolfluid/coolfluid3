import sys
# sys.path.append('/data/scholl/coolfluid3/build/dso')
sys.path.append('/home/sebastian/coolfluid3/build/dso')
import coolfluid as cf

# Global configuration
cf.Core.environment().options().configure_option('log_level', 4)

# setup a model
model = cf.Core.root().create_component('ScalarModel', 'cf3.solver.ModelUnsteady')
model.setup(solver_builder = 'cf3.UFEM.ScalarAdvection', physics_builder = 'cf3.physics.DynamicModel')
solver = model.get_child('ScalarAdvection')
domain = model.get_child('Domain')

# Generate a channel mesh
generator = domain.create_component('generator', 'cf3.mesh.SimpleMeshGenerator')
mesh = domain.create_component('mesh', 'cf3.mesh.Mesh')

generator.options().configure_option('mesh', mesh.uri());
generator.options().configure_option('lengths', [1.]);
generator.options().configure_option('nb_cells', [10]);
generator.execute()

# lss setup
lss = model.create_component('LSS', 'cf3.math.LSS.System')
lss.options().configure_option('solver', 'Trilinos');
solver.options().configure_option('lss', lss)
# old: lss.get_child('Matrix').options().configure_option('settings_file', sys.argv[2]);
lss.get_child('Matrix').options().configure_option('settings_file', 'solver.xml')

#initial conditions and properties
solver.options().configure_option('density', 1.)
solver.options().configure_option('reference_velocity', 1.)
solver.options().configure_option('dynamic_viscosity', 1.)
solver.options().configure_option('initial_scalar', 1.)
solver.options().configure_option('scalar_coefficient', 1.)
solver.options().configure_option('initial_velocity', [0.])

# Boundary conditions
bc = solver.get_child('TimeLoop').get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'xneg', variable_name = 'Scalar').options().configure_option('value', 1.)
bc.add_constant_bc(region_name = 'xpos', variable_name = 'Scalar').options().configure_option('value', 0.)
#bc.get_child('BCleftScalar').options().configure_option('value', 0)
#bc.get_child('BCrightScalar').options().configure_option('value', 1)

# Time setup
time = model.get_child('Time')
time.options().configure_option('time_step', 1.)
time.options().configure_option('end_time', 10.)

# Setup a time series write
final_end_time = 10.
save_interval = 1.
current_end_time = 0.
iteration = 0
while current_end_time < final_end_time:
  current_end_time += save_interval
  time.options().configure_option('end_time', current_end_time)
  model.simulate()
  domain.write_mesh(cf.URI('atest-scalaradvection-' +str(iteration) + '.msh'))
  iteration += 1
  if iteration == 1:
    solver.options().configure_option('disabled_actions', ['Initialize'])

# print timings
model.print_timing_tree()

