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
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Create a component to manage initial conditions
ic = solver.create_initial_conditions()

# Add the Navier-Stokes solver as an unsteady solver
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
ns_solver.options().configure_option('use_specializations', False)

# Add the SpalartAllmaras turbulence model solver(satm)
satm = solver.add_unsteady_solver('cf3.UFEM.SpalartAllmaras')

#Load mesh
mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

ns_solver.options().configure_option('regions', [mesh.access_component('topology').uri()])
ns_solver.options().configure_option('use_specializations', False)
satm.options().configure_option('regions', [mesh.access_component('topology').uri()])

# lss setup
lss = ns_solver.create_lss('cf3.math.LSS.TrilinosFEVbrMatrix')
lss.get_child('Matrix').options().configure_option('settings_file', sys.argv[2])
#LSS for Spalart-Allmaras turbulence model
satm_lss = satm.create_lss('cf3.math.LSS.TrilinosFEVbrMatrix')
satm_lss.get_child('Matrix').options().configure_option('settings_file', sys.argv[2])

u_in = [1., 0.]
u_wall = [0., 0.]
NU_in = 0.0001
NU_wall = 0.

# Add initial conditions for the Navier-Stokes solver, which uses 'solution' as a tag for its solution fields
ic_ns = ic.create_initial_condition('navier_stokes_solution')
# Initial advection velocity and its previous values, using linearized_velocity as tag
ic_linearized_vel = ic.create_initial_condition('linearized_velocity')

ic_ns.options().configure_option('Velocity', u_in)
ic_linearized_vel.options().configure_option('AdvectionVelocity', u_in)
ic_linearized_vel.options().configure_option('AdvectionVelocity1', u_in)
ic_linearized_vel.options().configure_option('AdvectionVelocity2', u_in)
ic_linearized_vel.options().configure_option('AdvectionVelocity3', u_in)

# Initial conditions for the spalart allmaras solver
ic_NU = ic.create_initial_condition('spalart_allmaras_solution')

ic_ns.options().configure_option('regions',[mesh.access_component('topology').uri()])
ic_linearized_vel.options().configure_option('regions', [mesh.access_component('topology').uri()])
ic_NU.options().configure_option('regions',[mesh.access_component('topology').uri()])

#initial conditions and properties
physics.options().configure_option('density', 1000.)
physics.options().configure_option('dynamic_viscosity', 10.)
physics.options().configure_option('reference_velocity', u_in[0])

# Boundary conditions for Navier-Stokes
bc = ns_solver.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'in', variable_name = 'Velocity').options().configure_option('value', u_in)
bc.add_constant_bc(region_name = 'symm', variable_name = 'Velocity').options().configure_option('value',  u_in)
bc.add_constant_bc(region_name = 'wall', variable_name = 'Velocity').options().configure_option('value',  u_wall)
bc.add_constant_bc(region_name = 'out', variable_name = 'Pressure').options().configure_option('value',  0.)

# Boundary conditions for Spalart-Allmaras
bc = satm.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'in', variable_name = 'TurbulentViscosity').options().configure_option('value', NU_in)
bc.add_constant_bc(region_name = 'symm', variable_name = 'TurbulentViscosity').options().configure_option('value',  NU_in)
bc.add_constant_bc(region_name = 'wall', variable_name = 'TurbulentViscosity').options().configure_option('value',  NU_wall)
bc.add_constant_bc(region_name = 'out', variable_name = 'TurbulentViscosity').options().configure_option('value',  NU_in)

# Boundary conditions
#bc = ns_solver.get_child('BoundaryConditions')
#bc.add_constant_bc(region_name = 'in', variable_name = 'Velocity')
#bc.add_constant_bc(region_name = 'symm', variable_name = 'Velocity')
#bc.add_constant_bc(region_name = 'wall', variable_name = 'Velocity')
#bc.add_constant_bc(region_name = 'out', variable_name = 'Pressure')
#bc.get_child('BCinVelocity').options().configure_option('value', u_in)
#bc.get_child('BCsymmVelocity').options().configure_option('value', u_in)
#bc.get_child('BCwallVelocity').options().configure_option('value', [0., 0.])
#bc.get_child('BCoutPressure').options().configure_option('value', 0.)

# Time setup
time = model.create_time()
time.options().configure_option('time_step', 0.01)

# Setup a time series write
final_end_time = 0.1
save_interval = 0.01
current_end_time = 0.
iteration = 0
while current_end_time < final_end_time:
  current_end_time += save_interval
  time.options().configure_option('end_time', current_end_time)
  model.simulate()
  domain.write_mesh(cf.URI('atest-ufem-navier-stokes-cylinder2d_output-' +str(iteration) + '.pvtu'))
  iteration += 1
  if iteration == 1:
    solver.options().configure_option('disabled_actions', ['InitialConditions'])

# print timings
model.print_timing_tree()
