import sys
# sys.path.append('/data/scholl/coolfluid3/build/dso')
sys.path.append('/home/sebastian/coolfluid3/build/dso')
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global confifuration
env.options().set('assertion_throws', False)
env.options().set('assertion_backtrace', False)
env.options().set('exception_backtrace', False)
env.options().set('regist_signal_handlers', False)
env.options().set('log_level', 4)

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')

model.setup(solver_builder = 'cf3.UFEM.NavierStokes', physics_builder = 'cf3.physics.DynamicModel')

solver = model.get_child('NavierStokes')
solver = model.get_child('ScalarAdvection')

domain = model.get_child('Domain')
domain.create_component('neuReader', 'cf3.mesh.neu.Reader')

# Generate a channel mesh
domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

# lss setup
lss = model.create_component('LSS', 'cf3.math.LSS.System')
lss.options().set('solver', 'Trilinos')
solver.options().set('lss', lss)
lss.get_child('Matrix').options().set('settings_file', sys.argv[2])

u_in = [2., 0.]

#initial conditions and properties
solver.options().set('density', 1000.)
solver.options().set('dynamic_viscosity', 10.)
solver.options().set('initial_velocity', u_in)
solver.options().set('reference_velocity', u_in[0])

# Boundary conditions
bc = solver.get_child('TimeLoop').get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'in', variable_name = 'Velocity')
bc.add_constant_bc(region_name = 'symm', variable_name = 'Velocity')
bc.add_constant_bc(region_name = 'wall', variable_name = 'Velocity')
bc.add_constant_bc(region_name = 'out', variable_name = 'Pressure')
bc.get_child('BCinVelocity').options().set('value', u_in)
bc.get_child('BCsymmVelocity').options().set('value', u_in)
bc.get_child('BCwallVelocity').options().set('value', [0., 0.])
bc.get_child('BCoutPressure').options().set('value', 0.)

# Time setup
time = model.get_child('Time')
time.options().set('time_step', 0.1)

# Setup a time series write
final_end_time = 10.
save_interval = 1.
current_end_time = 0.
iteration = 0
while current_end_time < final_end_time:
  current_end_time += save_interval
  time.options().set('end_time', current_end_time)
  model.simulate()
  domain.write_mesh(cf.URI('atest-ufem-navier-stokes-cylinder2d_output-' +str(iteration) + '.pvtu'))
  iteration += 1
  if iteration == 1:
    solver.options().set('disabled_actions', ['Initialize'])

# print timings
model.print_timing_tree()
