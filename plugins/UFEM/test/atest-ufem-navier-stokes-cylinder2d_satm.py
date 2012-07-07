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
ns_solver.options.use_specializations = False

# Add the SpalartAllmaras turbulence model solver(satm)
satm = solver.add_unsteady_solver('cf3.UFEM.SpalartAllmaras')

#Load mesh
mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

ns_solver.regions = [mesh.topology.uri()]
satm.regions = [mesh.topology.uri()]

# lss setup
lss = ns_solver.create_lss('cf3.math.LSS.TrilinosFEVbrMatrix')
lss.Matrix.settings_file = sys.argv[2]
#LSS for Spalart-Allmaras turbulence model
satm_lss = satm.create_lss('cf3.math.LSS.TrilinosFEVbrMatrix')
satm_lss.Matrix.settings_file = sys.argv[2]

u_in = [1., 0.]
u_wall = [0., 0.]
NU_in = 0.0001
NU_wall = 0.

#initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = u_in
solver.InitialConditions.spalart_allmaras_solution.TurbulentViscosity = NU_in

#physical properties
physics.density = 1.2
physics.dynamic_viscosity = 1.7894e-5
physics.reference_velocity = u_in[0]

# Boundary conditions for Navier-Stokes
bc = ns_solver.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'in', variable_name = 'Velocity').value = u_in
bc.add_constant_bc(region_name = 'symm', variable_name = 'Velocity').value =  u_in
bc.add_constant_bc(region_name = 'wall', variable_name = 'Velocity').value =  u_wall
bc.add_constant_bc(region_name = 'out', variable_name = 'Pressure').value =  0.

# Boundary conditions for Spalart-Allmaras
bc = satm.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'in', variable_name = 'TurbulentViscosity').value = NU_in
bc.add_constant_bc(region_name = 'symm', variable_name = 'TurbulentViscosity').value =  NU_in
bc.add_constant_bc(region_name = 'wall', variable_name = 'TurbulentViscosity').value =  NU_wall
bc.add_constant_bc(region_name = 'out', variable_name = 'TurbulentViscosity').value =  NU_in

# Time setup
time = model.create_time()
time.time_step = 0.1

# Setup a time series write
final_end_time = 2.
save_interval = 0.1
time.end_time = 0.
while time.end_time < final_end_time:
  time.end_time += save_interval
  model.simulate()
  domain.write_mesh(cf.URI('navier-stokes-cylinder2d_satm_output-' +str(int(time.end_time/save_interval)) + '.pvtu'))
  solver.options.disabled_actions = ['InitialConditions']

# print timings
model.print_timing_tree()
