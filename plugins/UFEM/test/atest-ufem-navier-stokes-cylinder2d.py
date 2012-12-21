import sys
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global confifuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 4

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the Navier-Stokes solver as an unsteady solver
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
#ns_solver.options.use_specializations = False

#Load mesh
mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

ns_solver.regions = [mesh.topology.uri()]

u_in = [2., 0.]

#initial condition for the velocity. Unset variables (i.e. the pressure) default to zero
solver.InitialConditions.navier_stokes_solution.Velocity = u_in
# Physical constants
physics.options().set('density', 1000.)
physics.options().set('dynamic_viscosity', 10.)
physics.options().set('reference_velocity', u_in[0])

# Boundary conditions
bc = ns_solver.BoundaryConditions
bc.add_constant_bc(region_name = 'in', variable_name = 'Velocity').value = u_in
bc.add_constant_bc(region_name = 'symm', variable_name = 'Velocity').value = u_in
bc.add_constant_bc(region_name = 'wall', variable_name = 'Velocity').value = [0., 0.]
bc.add_constant_bc(region_name = 'out', variable_name = 'Pressure').value = 0.

# Time setup
time = model.create_time()
time.options().set('time_step', 0.1)

# Setup a time series write
final_end_time = 2.
save_interval = 0.5
current_end_time = 0.
iteration = 0
while current_end_time < final_end_time:
  current_end_time += save_interval
  time.options().set('end_time', current_end_time)
  model.simulate()
  domain.write_mesh(cf.URI('navier-stokes-cylinder2d_output-' +str(iteration) + '.pvtu'))
  iteration += 1
  if iteration == 1:
    solver.options().set('disabled_actions', ['InitialConditions'])

# print timings
model.print_timing_tree()
