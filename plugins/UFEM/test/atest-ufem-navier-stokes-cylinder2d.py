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
env.configure_option('log_level', 4)

# setup a model
model = root.create_component('NavierStokes', 'CF.Solver.CModelUnsteady')
model.setup(solver_builder = 'CF.UFEM.NavierStokes', physics_builder = 'CF.Physics.DynamicModel')
solver = model.get_child('NavierStokes')
domain = model.get_child('Domain')

# Generate a channel mesh
domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

# lss setup
lss = model.create_component('LSS', 'CF.math.LSS.System')
lss.configure_option('solver', 'Trilinos')
solver.configure_option('lss', lss.uri())
lss.get_child('Matrix').configure_option('settings_file', sys.argv[2])

u_in = cf.RealVector(2)
u_in[0] = 2.
u_in[1] = 0.

u_wall = cf.RealVector(2)
u_wall[0] = 0.
u_wall[1] = 0.

#initial conditions and properties
solver.configure_option('density', 1000.)
solver.configure_option('dynamic_viscosity', 10.)
solver.configure_option('initial_velocity', u_in)
solver.configure_option('reference_velocity', u_in[0])

# Boundary conditions
bc = solver.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'in', variable_name = 'Velocity')
bc.add_constant_bc(region_name = 'symm', variable_name = 'Velocity')
bc.add_constant_bc(region_name = 'wall', variable_name = 'Velocity')
bc.add_constant_bc(region_name = 'out', variable_name = 'Pressure')
bc.get_child('BCinVelocity').configure_option('value', u_in)
bc.get_child('BCsymmVelocity').configure_option('value', u_in)
bc.get_child('BCwallVelocity').configure_option('value', u_wall)
bc.get_child('BCoutPressure').configure_option('value', 0.)

# Time setup
time = model.get_child('Time')
time.configure_option('time_step', 0.1)

# dummy writer (to load the library)
domain.create_component('VTKwriter', 'CF.Mesh.VTKXML.CWriter')

# Setup a time series write
final_end_time = 10.
save_interval = 1.
current_end_time = 0.
iteration = 0
while current_end_time < final_end_time:
  current_end_time += save_interval
  time.configure_option('end_time', current_end_time)
  model.simulate()
  domain.write_mesh(cf.URI('atest-ufem-navier-stokes-cylinder2d_output-' +str(iteration) + '.pvtu'))
  iteration += 1
  if iteration == 1:
    solver.disable_action('InitializePressure')
    solver.disable_action('InitializeVelocity')
    solver.disable_action('InitializeU1')
    solver.disable_action('InitializeU2')
    solver.disable_action('InitializeU3')

# print timings
model.print_timing_tree()
