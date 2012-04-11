import sys
#sys.path.append('/data/scholl/coolfluid3/build/dso')
# sys.path.append('/home/sebastian/coolfluid3/build/dso')
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

## Global confifuration
#env.options().configure_option('assertion_throws', False)
#env.options().configure_option('assertion_backtrace', False)
#env.options().configure_option('exception_backtrace', False)
#env.options().configure_option('regist_signal_handlers', False)
#env.options().configure_option('log_level', 4)

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')

model.setup(solver_builder = 'cf3.UFEM.NavierStokes', physics_builder = 'cf3.physics.DynamicModel')

solver = model.get_child('NavierStokes')
#advection_solver = model.create_component('ScalarAdvection', 'cf3.UFEM.ScalarAdvection')
model.print_tree()

phys_model = model.get_child('DynamicModel')
#advection_solver.configure_option_recursively('physical_model', phys_model)

domain = model.get_child('Domain')
domain.create_component('neuReader', 'cf3.mesh.neu.Reader')

# Load mesh
domain.load_mesh(file = cf.URI('flatplate2d.neu'), name = 'mesh')

# domain.load_mesh(file = cf.URI('/home/sebastian/gmsh/flatplate1.msh'), name = 'Mesh')
# domain.load_mesh(file = cf.URI('/home/sebastian/gmsh/ring2d-tris.neu'), name = 'mesh')

# lss setup
lss = model.create_component('LSS', 'cf3.math.LSS.System')
lss.options().configure_option('solver', 'Trilinos')
solver.options().configure_option('lss', lss)
lss.get_child('Matrix').options().configure_option('settings_file','solver.xml')

# lss setup
#advection_lss = advection_solver.create_component('LSS', 'cf3.math.LSS.System')
#advection_lss.options().configure_option('solver', 'Trilinos')
#advection_solver.options().configure_option('lss', lss)
#lss.get_child('Matrix').options().configure_option('settings_file','solver.xml')

u_in = [2., 0.]
u_ref = [2., 0.]
u_wall = [0., 0.]
#phi_in = 100
#phi_ref = 100
#phi_wall = 200

#initial conditions and properties for Navier-Stokes
solver.options().configure_option('density', 1000.)
solver.options().configure_option('dynamic_viscosity', 10.)
solver.options().configure_option('initial_velocity', u_in)
solver.options().configure_option('reference_velocity', u_in[0])

#initial conditions and properties for Scalar-Advection
#advection_solver.options().configure_option('initial_scalar', 0.)
#advection_solver.options().configure_option('density', 1.2)
#advection_solver.options().configure_option('initial_velocity', u_in)
#advection_solver.options().configure_option('reference_velocity', u_ref[0])
#advection_solver.options().configure_option('dynamic_viscosity', 1.7894e-5)
#advection_solver.options().configure_option('scalar_coefficient', 1.)

# Boundary conditions
bc = solver.get_child('TimeLoop').get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Velocity').options().configure_option('value', u_in)
bc.add_constant_bc(region_name = 'bottom1', variable_name = 'Velocity').options().configure_option('value',  u_wall)
bc.add_constant_bc(region_name = 'bottom2', variable_name = 'Velocity').options().configure_option('value',  u_wall)
bc.add_constant_bc(region_name = 'bottom3', variable_name = 'Velocity').options().configure_option('value',  u_in)
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Pressure').options().configure_option('value', 1)
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').options().configure_option('value', u_in)

# Boundary conditions
#bc = advection_solver.get_child('TimeLoop').get_child('BoundaryConditions')
#bc.add_constant_bc(region_name = 'inlet', variable_name = 'Scalar').options().configure_option('value', phi_in)
#bc.add_constant_bc(region_name = 'bottom1', variable_name = 'Scalar').options().configure_option('value',  phi_wall)
#bc.add_constant_bc(region_name = 'bottom2', variable_name = 'Scalar').options().configure_option('value',  phi_in)
#bc.add_constant_bc(region_name = 'bottom3', variable_name = 'Scalar').options().configure_option('value',  phi_in)
##bc.add_constant_bc(region_name = 'outlet', variable_name = 'Scalar').options().configure_option('value', 1)
#bc.add_constant_bc(region_name = 'top', variable_name = 'Scalar').options().configure_option('value', phi_in)

#bc.add_constant_bc(region_name = 'solid', variable_name = 'Velocity').options().configure_option('value',  u_wall)

#bc.get_child('BCinVelocity').options().configure_option('value', u_in)
#bc.get_child('BCsymmVelocity').options().configure_option('value', u_in)
#bc.get_child('BCwallVelocity').options().configure_option('value', [0., 0.])
#bc.get_child('BCoutPressure').options().configure_option('value', 0.)

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
  domain.write_mesh(cf.URI('atest-ufem-flatplate2d_output-' +str(iteration) + '.pvtu'))
  iteration += 1
  if iteration == 1:
    solver.options().configure_option('disabled_actions', ['Initialize'])

# print timings
model.print_timing_tree()
