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
env.options().set('log_level', 4)

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Create a component to manage initial conditions
ic = solver.create_initial_conditions()

# Add the Navier-Stokes solver as an unsteady solver
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

#Load mesh
domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

# lss setup
lss = ns_solver.create_lss('cf3.math.LSS.TrilinosFEVbrMatrix')
lss.get_child('Matrix').options().set('settings_file', sys.argv[2])

u_in = [2., 0.]

# Add initial conditions for the Navier-Stokes solver, which uses 'solution' as a tag for its solution fields
ic_ns = ic.create_initial_condition('solution')
# Initial advection velocity and its previous values, using linearized_velocity as tag
ic_linearized_vel = ic.create_initial_condition('linearized_velocity')

ic_ns.options().set('Velocity', u_in)
ic_linearized_vel.options().set('AdvectionVelocity', u_in)
ic_linearized_vel.options().set('AdvectionVelocity1', u_in)
ic_linearized_vel.options().set('AdvectionVelocity2', u_in)
ic_linearized_vel.options().set('AdvectionVelocity3', u_in)


#initial conditions and properties
physics.options().set('density', 1000.)
physics.options().set('dynamic_viscosity', 10.)
physics.options().set('reference_velocity', u_in[0])

# Boundary conditions
bc = ns_solver.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'in', variable_name = 'Velocity')
bc.add_constant_bc(region_name = 'symm', variable_name = 'Velocity')
bc.add_constant_bc(region_name = 'wall', variable_name = 'Velocity')
bc.add_constant_bc(region_name = 'out', variable_name = 'Pressure')
bc.get_child('BCinVelocity').options().set('value', u_in)
bc.get_child('BCsymmVelocity').options().set('value', u_in)
bc.get_child('BCwallVelocity').options().set('value', [0., 0.])
bc.get_child('BCoutPressure').options().set('value', 0.)

# Time setup
time = model.create_time()
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
    solver.options().set('disabled_actions', ['InitialConditions'])

# print timings
model.print_timing_tree()
