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
env.options().configure_option('log_level', 4)

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
model.setup(solver_builder = 'cf3.UFEM.NavierStokes', physics_builder = 'cf3.physics.DynamicModel')
solver = model.get_child('NavierStokes')
domain = model.get_child('Domain')
domain.create_component('neuReader', 'cf3.mesh.neu.Reader')

# Generate a channel mesh
domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

# lss setup
lss = model.create_component('LSS', 'cf3.math.LSS.System')
lss.options().configure_option('solver', 'Trilinos')
solver.options().configure_option('lss', lss)
lss.get_child('Matrix').options().configure_option('settings_file', sys.argv[2])

u_in = [2., 0.]

#initial conditions and properties
solver.options().configure_option('density', 1000.)
solver.options().configure_option('dynamic_viscosity', 10.)
solver.options().configure_option('initial_velocity', u_in)
solver.options().configure_option('reference_velocity', u_in[0])

# Boundary conditions
bc = solver.get_child('TimeLoop').get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'in', variable_name = 'Velocity')
bc.add_constant_bc(region_name = 'symm', variable_name = 'Velocity')
bc.add_constant_bc(region_name = 'wall', variable_name = 'Velocity')
bc.add_constant_bc(region_name = 'out', variable_name = 'Pressure')
bc.get_child('BCinVelocity').options().configure_option('value', u_in)
bc.get_child('BCsymmVelocity').options().configure_option('value', u_in)
bc.get_child('BCwallVelocity').options().configure_option('value', [0., 0.])
bc.get_child('BCoutPressure').options().configure_option('value', 0.)

# Time setup
time = model.get_child('Time')
time.options().configure_option('time_step', 0.1)

# Setup a time series write
final_end_time = 10.
save_interval = 1.
current_end_time = 0.
iteration = 0
while current_end_time < final_end_time:
  current_end_time += save_interval
  time.options().configure_option('end_time', current_end_time)
  model.simulate()
  domain.write_mesh(cf.URI('atest-ufem-navier-stokes-cylinder2d_output-' +str(iteration) + '.pvtu'))
  iteration += 1
  if iteration == 1:
    solver.options().configure_option('disabled_actions', ['Initialize'])

# print timings
model.print_timing_tree()
